using System;
using Zene.Structs;

namespace Zene.Audio
{
    public delegate double WaveShape(double phase);
    
    public class Oscillator : IAudioSource
    {
        public Oscillator(double freq, WaveShape shape)
        {
            Frequency = freq;
            Shape = shape;
        }
        
        public bool Stereo => false;
        
        public double Frequency { get; set; }
        public WaveShape Shape { get; set; }
        
        private double _oldTime;
        protected double _phase;
        public virtual Vector2 GetSample(double time)
        {
            _phase += ((time - _oldTime) * Frequency);
            _oldTime = time;
            
            return new Vector2(Shape(_phase));
        }
        
        public void Reset() { _phase = 0; }
        
        private static double SinFunc(double phase) => Math.Sin(phase * 2d * Math.PI);
        public static WaveShape Sin { get; } = SinFunc;
        
        private static double PulseFunc(double phase) => (((int)(phase * 2d) % 2) * 2d) - 1d;
        public static WaveShape Pulse { get; } = PulseFunc;
        
        private static double SawFunc(double phase) => ((phase - Math.Floor(phase)) * 2d) - 1d;
        public static WaveShape Saw { get; } = SawFunc;
        
        private static double TriangleFunc(double phase) => (Math.Abs(phase + 0.25 - Math.Floor(phase + 0.75)) * 4) - 1d;
        public static WaveShape Triangle { get; } = TriangleFunc;
    }
}