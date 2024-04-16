using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Zene.Structs;

namespace Zene.Audio
{
    public class AudioSystem : IAudioSystem, IDisposable
    {
        public unsafe AudioSystem(AudioDevice device, int blockSize)
        {
            if (!device.Output)
            {
                throw new Exception();
            }
            
            _handle = AUDIO.CreateAudioSystem(device._handle, (uint)blockSize);
            _callback = Callback;
            AUDIO.SetASCallback(_handle, _callback);
            
            Stereo = AUDIO.GetASNumChannels(_handle) > 1;
            _tOffset = 1d / SampleRate;
        }
        
        private IntPtr _handle;
        private readonly AUDIO.Callback _callback;
        
        public bool Stereo { get; }
        public List<IAudioSource> Sources { get; } = new List<IAudioSource>();
        
        public long SampleRate => (long)AUDIO.GetASSampleRate(_handle);
        public double Gain { get; set; }
        
        public bool Running => AUDIO.IsASRunning(_handle);
        
        public AudioDevice Device { get; }
        
        public bool Start() => AUDIO.StartAS(_handle);
        public void Stop() => AUDIO.StopAS(_handle);
        
        private readonly double _tOffset;
        private double _currentTime;
        private unsafe void Callback(float* block, uint size, uint channels)
        {
            ReadOnlySpan<IAudioSource> srcs = CollectionsMarshal.AsSpan(Sources);
            int nSrcs = srcs.Length;
            for (int f = 0; f < size; f++)
            {
                Vector2 sample = Vector2.Zero;
                for (int s = 0; s < nSrcs; s++)
                {
                    IAudioSource src = srcs[s];
                    Vector2 srcV = src.GetSample(_currentTime);
                    
                    if (src.Stereo)
                    {
                        sample += srcV;
                        continue;
                    }
                    
                    sample += new Vector2(srcV.X);
                }
                
                _currentTime += _tOffset;
                
                sample *= Gain;
                sample = new Vector2(
                    Math.Clamp(sample.X, -1d, 1d),
                    Math.Clamp(sample.Y, -1d, 1d));
                
                if (channels < 2)
                {
                    block[f] = (float)sample.X;
                    continue;
                }
                
                block[f * channels] = (float)sample.X;
                block[(f * channels) + 1] = (float)sample.Y;
            }
        }
        
        private bool _disposed = false;
        public void Dispose()
        {
            if (_disposed) { return; }
            _disposed = true;
            
            AUDIO.DeleteAudioSystem(_handle);
            
            GC.SuppressFinalize(this);
        }
    }
}