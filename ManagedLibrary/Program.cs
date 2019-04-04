using Newtonsoft.Json;
using System;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Threading;
using Microsoft.CSharp;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis;
using System.IO;
using System.Runtime.Loader;

namespace ManagedLibrary
{
    // Sample managed code for the host to call
    public class ManagedWorker
    {
        // This assembly is being built as an exe as a simple way to
        // get .NET Core runtime libraries deployed (`dotnet publish` will
        // publish .NET Core libraries for exes). Therefore, this assembly
        // requires an entry point method even though it is unused.
        public delegate int ReportProgressFunction(int progress);
        struct Objekt
        {
            public Objekt(string name, int number)
            {
                Name = name;
                Number = number;
            }
            public string Name { get; }
            public int Number { get; }
        }

        // This test method doesn't actually do anything, it just takes some input parameters,
        // waits (in a loop) for a bit, invoking the callback function periodically, and
        // then returns a string version of the double[] passed in.
        [return: MarshalAs(UnmanagedType.U1)]
        public static bool Load([MarshalAs(UnmanagedType.LPStr)] string assemblyName)
        {
            try
            {

                var serialized = JsonConvert.SerializeObject(new Objekt("Uwe", 234));

                Console.WriteLine("Starting");
                var se = new Stopwatch();
                se.Start();
                for (var i = 0; i < 1_000_000; i++)
                    serialized = JsonConvert.SerializeObject(new Objekt("Uwe", 234));
                var elapsed = se.Elapsed;

                var dotnetCoreDirectory = Path.GetDirectoryName(typeof(object).GetTypeInfo().Assembly.Location);


                var compilation = CSharpCompilation.Create("a")
                    .WithOptions(new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary))
                    .AddReferences(
                        MetadataReference.CreateFromFile(typeof(string).GetTypeInfo().Assembly.Location),
                        MetadataReference.CreateFromFile(typeof(object).GetTypeInfo().Assembly.Location),
                        MetadataReference.CreateFromFile(typeof(Console).GetTypeInfo().Assembly.Location),
                        MetadataReference.CreateFromFile(Path.Combine(dotnetCoreDirectory, "System.Runtime.dll")))
                    .AddSyntaxTrees(CSharpSyntaxTree.ParseText(
                        @"
using System;

public static class C
{
    public static void M()
    {
        Console.WriteLine(""Hello Roslyn."");
    }
}"));

                using (var memoryStream = new MemoryStream())
                {
                    var emitResult = compilation.Emit(memoryStream);
                    if (emitResult.Success)
                    {
                        memoryStream.Seek(0, SeekOrigin.Begin);

                        var context = AssemblyLoadContext.Default;
                        var assembly = context.LoadFromStream(memoryStream);

                        assembly.GetType("ClassName").GetMethod("MethodName").Invoke(null, null);
                    }
                    else
                    {
                        var diags = emitResult.Diagnostics;
                    }
                }


                //var fileName = "a.dll";

                //compilation.Emit(fileName);

                //var a = AssemblyLoadContext.Default.LoadFromAssemblyPath(Path.GetFullPath(fileName));

                //a.GetType("C").GetMethod("M").Invoke(null, null);
                //Console.WriteLine($"End: {elapsed}");
                //Console.ReadLine();



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
