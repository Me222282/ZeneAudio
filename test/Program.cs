using System;
using System.Threading;
using Zene.Audio;

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
                
                float v = (float)Math.Sin(((Math.PI * _phase * 2d) / 44100d) * _frequency) * 0.3f;
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
            IntPtr* array = AUDIO.GetOutputs(dc, out int size);
            IntPtr audioSystem = AUDIO.CreateAudioSystem(device, 1024);
            
            int[] notes = new int[8] {
                27, 31, 34, 38, 39, 38, 34, 31
            };
            int bpm = 140 * 2;
            
            AUDIO.SetASCallback(audioSystem, Func);
            
            AUDIO.StartAS(audioSystem);
            for (int i = 0; i < 8; i++)
            {
                for (int j = 0; j < 8; j++)
                {
                    _frequency = NoteToFreq(notes[j]);
                    Thread.Sleep(60_000 / bpm);
                }
            }
            AUDIO.StopAS(audioSystem);
            
            AUDIO.DeleteAudioSystem(audioSystem);
            AUDIO.DeleteInitialiser(dc);
        }
    }
}
