using System;
using Zene.Structs;

namespace Zene.Audio
{
    public interface IAudioSource
    {
        public bool Stereo { get; }
        
        public Vector2 GetSample(double time);
    }
}