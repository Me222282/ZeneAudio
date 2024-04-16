using System;
using System.Collections.Generic;

namespace Zene.Audio
{
    public interface IAudioPipeline : IAudioSource
    {
        public double Gain { get; set; }
        
        public IAudioSource Source { get; }
        public List<IAudioEffect> Effects { get; }
    }
}