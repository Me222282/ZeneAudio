using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security;
using Zene.Structs;

namespace Zene.Audio
{
    public interface IAudioPipeline : IAudioSource
    {
        public double Gain { get; set; }
        
        public IAudioSource Source { get; }
        public List<IAudioEffect> Effects { get; }
    }
}