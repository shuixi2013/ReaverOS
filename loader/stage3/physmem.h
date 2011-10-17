#ifndef __physmem_h__
#define __physmem_h__

// there is soooo much things in this namespace...
namespace PhysMemory
{
    // placement address. self-explanatory
    extern int PlacementAddress;
    
    class Manager
    {
    public:
        // and this one returns address for object of size = iSize
        static void * Place(int iSize);
    };
}

#endif