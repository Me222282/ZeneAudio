using System;

namespace Zene.Audio
{
    public static class Initialiser
    {
        private static IntPtr _handle;
        
        public static AudioDevice DefaultOutput { get; }
        public static AudioDevice[] Outputs { get; }
        
        public static AudioDevice DefaultInput { get; }
        public static AudioDevice[] Inputs { get; }
        
        static unsafe Initialiser()
        {
            AUDIO.Initialise(false, out _handle);
            
            // Outputs
            IntPtr defOut = AUDIO.GetDefaultOutput(_handle);
            DefaultOutput = new AudioDevice(defOut, true);
            
            IntPtr* outs = AUDIO.GetOutputs(_handle, out int numOuts);
            Outputs = new AudioDevice[numOuts];
            for (int i = 0; i < numOuts; i++)
            {
                IntPtr ptr = outs[i];
                if (ptr == IntPtr.Zero)
                {
                    Outputs[i] = null;
                    continue;
                }
                Outputs[i] = new AudioDevice(ptr, true);
            }
            
            // Inputs
            IntPtr defIn = AUDIO.GetDefaultOutput(_handle);
            DefaultInput = new AudioDevice(defIn, false);
            
            IntPtr* ins = AUDIO.GetOutputs(_handle, out int numIns);
            Inputs = new AudioDevice[numIns];
            for (int i = 0; i < numIns; i++)
            {
                IntPtr ptr = ins[i];
                if (ptr == IntPtr.Zero)
                {
                    Outputs[i] = null;
                    continue;
                }
                Inputs[i] = new AudioDevice(ptr, false);
            }
        }
        
        public static void Terminate()
        {
            AUDIO.DeleteInitialiser(_handle);
        }
    }
}
