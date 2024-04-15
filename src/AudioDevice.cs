using System;
using Zene.Structs;

namespace Zene.Audio
{
    public class AudioDevice
    {
        // TEMP
        public AudioDevice(IntPtr handle, bool output)
        {
            _handle = handle;
            Output = output;
        }
        
        internal IntPtr _handle;
        public bool Output { get; }
        
        public string Name { get; }
        public string Id { get; }
    }
}