using System;

namespace Zene.Audio
{
    public class AudioReader : IDisposable
    {
        public delegate void ReadAudio(Span<float> data, uint channels);
        
        public unsafe AudioReader(AudioDevice device, int blockSize)
        {
            if (device.Output)
            {
                throw new Exception();
            }
            
            _handle = AUDIO.CreateAudioReader(device._handle, (uint)blockSize);
            _callback = Callback;
            AUDIO.SetARCallback(_handle, _callback);
            
            Stereo = AUDIO.GetARNumChannels(_handle) > 1;
            // _tOffset = 1d / SampleRate;
        }
        
        private IntPtr _handle;
        private readonly AUDIO.Callback _callback;
        
        public bool Stereo { get; }
        
        public long SampleRate => (long)AUDIO.GetARSampleRate(_handle);
        
        public bool Running => AUDIO.IsARRunning(_handle);
        
        public AudioDevice Device { get; }
        
        public bool Start() => AUDIO.StartAR(_handle);
        public void Stop() => AUDIO.StopAR(_handle);
        
        public event ReadAudio OnAudio;
        
        // private readonly double _tOffset;
        // private double _currentTime;
        private unsafe void Callback(float* block, uint size, uint channels)
        {
            Span<float> span = new Span<float>(block, (int)(size * channels));

            OnAudio?.Invoke(span, channels);
        }
        
        private bool _disposed = false;
        public void Dispose()
        {
            if (_disposed) { return; }
            _disposed = true;
            
            AUDIO.DeleteAudioReader(_handle);
            
            GC.SuppressFinalize(this);
        }
    }
}