#ifndef _VALUE_H
#define _VALUE_H

namespace CacheSystem
{
	/**
	represents a generic cached value
	*/
	class Value
	{
	public:
		/**
		only defined to make the destructor virtual
		*/
		virtual ~Value(){}
	};
}

#endif