<h1>Collection of my C++ classes</h1>

This is a collection of my C++ classes that I think might come useful for someone or me in the future.

I started documenting and licensing the classes.

Detailed documentation of the listed classes can be found in the source code!

<h3>WinAPI / ProcessMemory.hpp</h3>
This class capable of reading and writing values through multi level pointers from any process's memory. The process can be opened by window title or executable name. <i>(The order of pointer offsets are the reverse of what CheatEngine shows)</i>

```cpp
ProcessMemory pm;
pm.OpenByExecutableName("process.exe");
//or pm.OpenByWindowTitle("My window");
int32_t value;
while (pm.IsValid())
{
	pm.ReadMemoryValue(value, pm.GetModuleAddr("process.exe") + 0x00ABCDEF, { 0x123, 0x234, 0x345, 0x456}); //Read multi level pointer
	pm.WriteMemoryValue<int32_t>(1111, pm.GetModuleAddr("example.dll") + 0x00123456); //Read value
}
pm.Close();
```

<h3>Misc / ScopedPtr.hpp</h3>
Scoped pointer implementation. The class' destructior takes care of the release of the memory. It's used to create memory leak free code.

```cpp
ScopedPtr<int> sptr(new int);
*sptr = 44;
std::cout << *sptr << std::endl;

ScopedPtr<int, true> sptrarray(new int[10000]);
sptrarray[100] = 44;
std::cout << sptrarray[100] << std::endl;
```
