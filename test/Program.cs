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
        
        static uint _phase = 0;
        static double _frequency = 0;
        static unsafe void Func(float* block, uint size, uint nChannels)
        {
            /*
            uint32_t avai = 0;
            float* source = readAudioSource(audioReader, size, &avai);
            // 2 channels
            avai *= 2;
            
            uint32_t readPoint = 0;*/
            for (uint i = 0; i < size; i++)
            {
                _phase++;
                
                float v = (float)Math.Sin(((Math.PI * _phase * 2d) / 44100d) * _frequency * 2d) * 0.3f;
                // Read twice for both channels
                /*float v = source[readPoint];
                readPoint++;
                v += source[readPoint];
                readPoint++;*/
                
                //block[i] = sin(((M_PI * i * 2) / 44100) * 200) * 0.2f;
                block[i * 2] = v;
                block[(i * 2) + 1] = v;
            }
        }
        
        static unsafe void Main(string[] args)
        {
            IntPtr dc = IntPtr.Zero;
            bool g = AUDIO.Initialise(false, out dc);
            IntPtr device = AUDIO.GetDefaultOutput(dc);
            AudioSystem system = new AudioSystem(new AudioDevice(device, true), 1024);
            
            Source src = new Source();
            system.Sources.Add(src);
            
            system.Start();
            
            while (src.NTimes < 8)
            {
                Thread.Sleep(1000);
            }
            system.Stop();
            
            system.Dispose();
            AUDIO.DeleteInitialiser(dc);
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
