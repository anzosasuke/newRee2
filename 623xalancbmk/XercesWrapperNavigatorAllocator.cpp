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
#include "xalanc/XercesParserLiaison/XercesWrapperNavigatorAllocator.hpp"



XALAN_CPP_NAMESPACE_BEGIN



XercesWrapperNavigatorAllocator::XercesWrapperNavigatorAllocator(MemoryManagerType& theManager, size_type	theBlockCount) :
	m_allocator(theManager, theBlockCount)
{
}



XercesWrapperNavigatorAllocator::~XercesWrapperNavigatorAllocator()
{
}



XercesWrapperNavigatorAllocator::ObjectType*
XercesWrapperNavigatorAllocator::create(XercesDocumentWrapper*  theOwnerDocument)
{
	ObjectType* const	theBlock = m_allocator.allocateBlock();
	assert(theBlock != 0);

	new(theBlock) ObjectType(theOwnerDocument);

	m_allocator.commitAllocation(theBlock);

	return theBlock;
}



void 
XercesWrapperNavigatorAllocator::reset()
{
	m_allocator.reset();
}



XALAN_CPP_NAMESPACE_END
