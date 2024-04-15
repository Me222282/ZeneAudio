using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security;
using Zene.Structs;

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