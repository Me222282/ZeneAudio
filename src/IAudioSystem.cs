using System;
using System.Collections.Generic;

namespace Zene.Audio
{
    public interface IAudioSystem
    {
        public bool Stereo { get; }
        public long SampleRate { get; }
        public List<IAudioSource> Sources { get; }
        
        public bool Running { get; }
        
        public AudioDevice Device { get; }
        
        public bool Start();
        public void Stop();
    }
}