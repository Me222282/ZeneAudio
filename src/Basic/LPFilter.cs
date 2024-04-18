using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Zene.Structs;

namespace Zene.Audio
{
    public class LPFilter : IAudioEffect
    {
        public LPFilter(double freq, int stages)
        {
            _stages = new Stage[stages];
            _numStages = stages;
            CutOff = freq;
        }
        
        public bool Stereo => true;
        
        private double _freq;
        public double CutOff
        {
            get => _freq;
            set
            {
                _freq = value;
                _RC = 1d / (2d * Math.PI * value);
            }
        }
        
        public double Resonance { get; set; } = 0d;
        
        private double _RC;
        
        public int Stages
        {
            get => _numStages;
            set
            {
                // Need more memory - allocate and copy
                if (_stages.Length < value)
                {
                    Stage[] ns = new Stage[value];
                    Array.Copy(_stages, ns, _numStages);
                    _stages = ns;
                    return;
                }
                
                // Enough memory - set new limit
                _numStages = value;
                if (value <= _numStages) { return; }
                
                // Potentually reuseing old stages - clear that data
                Array.Clear(_stages, _numStages + 1, value - _numStages);
            }
        }
        
        private Vector2 _lastSample;
        private double _lastTime;
        private Stage[] _stages;
        private int _numStages;
        public Vector2 GetSample(double time, Vector2 source)
        {
            Vector2 v = source + (Resonance * _lastSample);
            double dt = time - _lastTime;
            double a = dt / (_RC + dt);
            
            ReadOnlySpan<Stage> s = _stages;
            for (int i = 0; i < _numStages; i++)
            {
                v = _stages[i].GetSample(time, a, v);
            }
            
            _lastSample = v;
            _lastTime = time;
            return v;
        }
        
        private struct Stage
        {
            private Vector2 _lastSample;
            
            public Vector2 GetSample(double time, double a, Vector2 source)
            {
                Vector2 v = (source * a) + ((1d - a) * _lastSample);
                _lastSample = v;
                return v;
            }
        }
    }
}