class MyClass
{
private:
	struct MyClassInitializer
	{
		MyClassInitializer()
		{
      std::cout << "My Class Initialized..." << std::endl;
			delete this;
		}
	} static* initializer;
};

MyClass::MyClassInitializer* MyClass::initializer = new MyClass::MyClassInitializer();
