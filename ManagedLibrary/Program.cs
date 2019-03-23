using System;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Threading;

namespace ManagedLibrary
{
    // Sample managed code for the host to call
    public class ManagedWorker
    {
        // This assembly is being built as an exe as a simple way to
        // get .NET Core runtime libraries deployed (`dotnet publish` will
        // publish .NET Core libraries for exes). Therefore, this assembly
        // requires an entry point method even though it is unused.
        public static void Main()
        {
            Console.WriteLine("This assembly is not meant to be run directly.");
            Console.WriteLine("Instead, please use the SampleHost process to load this assembly.");
        }

        public delegate int ReportProgressFunction(int progress);

        // This test method doesn't actually do anything, it just takes some input parameters,
        // waits (in a loop) for a bit, invoking the callback function periodically, and
        // then returns a string version of the double[] passed in.
        [return: MarshalAs(UnmanagedType.U1)]
        public static bool Load([MarshalAs(UnmanagedType.LPStr)] string assemblyName)
        {
            try
            {
                var assi = Assembly.Load("Standard");
                return true;
            }
            catch (Exception e)
            {
                Console.Error.WriteAsync($"Could not load assembly: {e}");
                return false;
            }
        }
    }
}
