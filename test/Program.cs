using System;
using System.Threading;
using Zene.Audio;
using Zene.Structs;

namespace test
{
    class Program
    {
        static double NoteToFreq(int n)
        {
            return 27.5f * Math.Pow(2.0f, n / 12.0f);
        }
        
        static unsafe void Main(string[] args)
        {
            AudioSystem system = new AudioSystem(Devices.DefaultOutput, 1024);
            
            Source src = new Source();
            system.Sources.Add(src);
            
            system.Start();
            
            while (src.NTimes < 8)
            {
                Thread.Sleep(1000);
            }
            system.Stop();
            
            system.Dispose();
            Devices.Terminate();
        }

        private class Source : Oscillator
        {
            public Source()
                : base(400, Oscillator.Sin)
            {
                Frequency = 2d * NoteToFreq(_notes[_noteIndex]);
            }
            
            private int[] _notes = new int[8] {
                27, 31, 34, 38, 39, 38, 34, 31
            };
            private int _bpm = 140 * 2;
            
            private int _noteIndex = 0;
            private double _tOffset = 0;
            
            public int NTimes { get; private set; }= 0;
            
            public override Vector2 GetSample(double time)
            {
                if ((((time - _tOffset) / 60d) * _bpm) >= 1)
                {
                    _tOffset = time;
                    _noteIndex++;
                    if (_noteIndex >= _notes.Length)
                    {
                        _noteIndex = 0;
                        NTimes++;
                    }
                    
                    Frequency = 2d * NoteToFreq(_notes[_noteIndex]);
                }
                
                return base.GetSample(time);
            }
        }
    }
}
