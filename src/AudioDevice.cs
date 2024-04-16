using System;
using Zene.Structs;

namespace Zene.Audio
{
    public class AudioDevice
    {
        internal AudioDevice(IntPtr handle, bool output)
        {
            _handle = handle;
            Output = output;
            // Add name and id setting
        }
        
        internal IntPtr _handle;
        public bool Output { get; }
        
        public string Name { get; }
        public string Id { get; }
    }
}