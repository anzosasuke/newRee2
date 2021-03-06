/*
 * Copyright 1999-2004 The Apache Software Foundation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Class header file.
#include "xalanc/XPath/XStringCachedAllocator.hpp"



XALAN_CPP_NAMESPACE_BEGIN



XStringCachedAllocator::XStringCachedAllocator(MemoryManagerType&  theManager, size_type	theBlockCount) :
	m_allocator(theManager, theBlockCount)
{
}



XStringCachedAllocator::~XStringCachedAllocator()
{
}



XStringCachedAllocator::string_type*
XStringCachedAllocator::createString(GetAndReleaseCachedString&		theValue) 
{
	string_type* const	theBlock = m_allocator.allocateBlock();
	assert(theBlock != 0);

    string_type* const	theResult = new(theBlock) string_type(theValue, m_allocator.getMemoryManager());

	m_allocator.commitAllocation(theBlock);

	return theResult;
}




bool 
XStringCachedAllocator::destroy(string_type*	theString)
{
	return m_allocator.destroyObject(theString);
}



void 
XStringCachedAllocator::reset()
{
	m_allocator.reset();
}



XALAN_CPP_NAMESPACE_END
