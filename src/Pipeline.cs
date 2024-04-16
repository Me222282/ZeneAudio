using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Zene.Structs;

namespace Zene.Audio
{
    public class Pipeline : IAudioPipeline
    {
        public Pipeline(IAudioSource source)
        {
            Source = source;
        }
        
        public double Gain { get; set; } = 1d;
        
        public IAudioSource Source { get; set; }
        public List<IAudioEffect> Effects { get; } = new List<IAudioEffect>();

        public bool Stereo => true;

        public Vector2 GetSample(double time)
        {
            Vector2 v = Source.GetSample(time);
            if (!Source.Stereo)
            {
                v = new Vector2(v.X);
            }
            
            ReadOnlySpan<IAudioEffect> effects = CollectionsMarshal.AsSpan(Effects);
            for (int i = 0; i < effects.Length; i++)
            {
                IAudioEffect effect = effects[i];
                v = effect.GetSample(time, v);
                if (!effect.Stereo)
                {
                    v = new Vector2(v.X);
                }
            }
            
            return v * Gain;
        }
    }
}