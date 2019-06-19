<h1>Collection of my C++ classes</h1>

This is a collection of my C++ classes that I think might come useful for someone or me in the future.

I started documenting and licensing the classes.

Detailed documentation of the listed classes can be found in the source code!

<h3>WinAPI / ProcessMemory.hpp</h3>
This class capable of reading and writing values through multi level pointers from any process's memory. The process can be opened by window title or executable name.

```cpp
ProcessMemory pm;
pm.OpenByExecutableName("process.exe");
int32_t value;
while (pm.IsValid())
{
	pm.ReadMemoryValue(value, pm.GetModuleAddr("process.exe") + 0x00ABCDEF, { 0x123, 0x234, 0x345, 0x456});
	pm.WriteMemoryValue<int32_t>(1111, pm.GetModuleAddr("process.exe") + 0x00123456);
}
pm.Close();
```
