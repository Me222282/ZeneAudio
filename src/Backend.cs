using System;
using System.Runtime.InteropServices;
using System.Security;

namespace Zene.Audio
{
    internal unsafe static class AUDIO
    {
#if WINDOWS
		private const string LinkLibrary = "deps/audio_wasapi";
#elif UNIX
		private const string LinkLibrary = "deps/audio_alsa.so";
#endif
    
        [UnmanagedFunctionPointer(CallingConvention.Cdecl), SuppressUnmanagedCodeSecurity]
        public delegate void Callback(float* block, uint size, uint channels);
        
        [DllImport(LinkLibrary, EntryPoint = "getASSampleRate", CallingConvention = CallingConvention.Cdecl)]
        public static extern ulong GetASSampleRate(IntPtr audioSys);
        [DllImport(LinkLibrary, EntryPoint = "getASBlockSize", CallingConvention = CallingConvention.Cdecl)]
        public static extern ulong GetASBlockSize(IntPtr audioSys);
        [DllImport(LinkLibrary, EntryPoint = "getASNumChannels", CallingConvention = CallingConvention.Cdecl)]
        public static extern ulong GetASNumChannels(IntPtr audioSys);
        
        [DllImport(LinkLibrary, EntryPoint = "startAS", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool StartAS(IntPtr audioSys);
        [DllImport(LinkLibrary, EntryPoint = "isASRunning", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool IsASRunning(IntPtr audioSys);
        [DllImport(LinkLibrary, EntryPoint = "stopAS", CallingConvention = CallingConvention.Cdecl)]
        public static extern void StopAS(IntPtr audioSys);
        
        [DllImport(LinkLibrary, EntryPoint = "setASCallback", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetASCallback(IntPtr audioSys, Callback callback);
        
        [DllImport(LinkLibrary, EntryPoint = "getAudioSystemDevice", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr GetAudioSystemDevice(IntPtr audioSys);
        [DllImport(LinkLibrary, EntryPoint = "createAudioSystem", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr CreateAudioSystem(IntPtr outputDevice, uint blockSize);
        [DllImport(LinkLibrary, EntryPoint = "deleteAudioSystem", CallingConvention = CallingConvention.Cdecl)]
        public static extern void DeleteAudioSystem(IntPtr audioSys);
        
        [DllImport(LinkLibrary, EntryPoint = "getDeviceName", CallingConvention = CallingConvention.Cdecl)]
        public static extern byte* GetDeviceName(IntPtr device, out uint size);
        [DllImport(LinkLibrary, EntryPoint = "getDeviceId", CallingConvention = CallingConvention.Cdecl)]
        public static extern char* GetDeviceId(IntPtr device, out uint size);
        
        [DllImport(LinkLibrary, EntryPoint = "initialise", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool Initialise(bool captures, out IntPtr deviceCollection);
        
        [DllImport(LinkLibrary, EntryPoint = "getDefaultOutput", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr GetDefaultOutput(IntPtr deviceCollection);
        [DllImport(LinkLibrary, EntryPoint = "getOutputs", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr* GetOutputs(IntPtr deviceCollection, out int size);
        
        [DllImport(LinkLibrary, EntryPoint = "getDefaultInput", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr GetDefaultInput(IntPtr deviceCollection);
        [DllImport(LinkLibrary, EntryPoint = "getInputs", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr* GetInputs(IntPtr deviceCollection, out int size);
        
        [DllImport(LinkLibrary, EntryPoint = "deleteInitialiser", CallingConvention = CallingConvention.Cdecl)]
        public static extern void DeleteInitialiser(IntPtr deviceCollection);
        
        
        [DllImport(LinkLibrary, EntryPoint = "getARSampleRate", CallingConvention = CallingConvention.Cdecl)]
        public static extern ulong GetARSampleRate(IntPtr audioRead);
        [DllImport(LinkLibrary, EntryPoint = "getARBlockSize", CallingConvention = CallingConvention.Cdecl)]
        public static extern ulong GetARBlockSize(IntPtr audioRead);
        [DllImport(LinkLibrary, EntryPoint = "getARNumChannels", CallingConvention = CallingConvention.Cdecl)]
        public static extern ulong GetARNumChannels(IntPtr audioRead);
        
        [DllImport(LinkLibrary, EntryPoint = "startAR", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool StartAR(IntPtr audioRead);
        [DllImport(LinkLibrary, EntryPoint = "isARRunning", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool IsARRunning(IntPtr audioRead);
        [DllImport(LinkLibrary, EntryPoint = "stopAR", CallingConvention = CallingConvention.Cdecl)]
        public static extern void StopAR(IntPtr audioRead);
        
        [DllImport(LinkLibrary, EntryPoint = "setARCallback", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetARCallback(IntPtr audioRead, Callback callback);
        
        [DllImport(LinkLibrary, EntryPoint = "getAudioReaderDevice", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr GetAudioReaderDevice(IntPtr audioRead);
        [DllImport(LinkLibrary, EntryPoint = "createAudioReader", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr CreateAudioReader(IntPtr inputDevice, uint blockSize);
        [DllImport(LinkLibrary, EntryPoint = "deleteAudioReader", CallingConvention = CallingConvention.Cdecl)]
        public static extern void DeleteAudioReader(IntPtr audioRead);
    }
}
