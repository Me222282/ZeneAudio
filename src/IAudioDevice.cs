using Zene.Structs;

namespace Zene.Audio
{
    public interface IAudioDevice
    {
        public bool Output { get; }
        
        public string Name { get; }
        public string Id { get; }
    }
}