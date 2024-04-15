using System;

namespace Zene.Audio
{
    public readonly ref struct Sample
    {
        public Sample(ReadOnlySpan<double> span)
        {
            _span = span;
        }
        
        private readonly ReadOnlySpan<double> _span;
        
        public int Channels => _span.Length;
        public double this[int channel]
        {
            get
            {
                if (channel >= _span.Length) { return 0d; }
                
                return _span[channel];
            }
        }
    }
}