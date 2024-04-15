using Zene.Structs;

namespace Zene.Audio
{
    public interface IAudioEffect
    {
        public bool Stereo { get; }
        
        public Vector2 GetSample(double time, Vector2 source);
    }
}