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
#include "xalanc/PlatformSupport/PrintWriter.hpp"



#include "xalanc/PlatformSupport/DOMStringHelper.hpp"
#include "xalanc/PlatformSupport/XalanUnicode.hpp"



XALAN_CPP_NAMESPACE_BEGIN



const XalanDOMChar	PrintWriter::s_trueString[] =
{
	XalanUnicode::charLetter_t,
	XalanUnicode::charLetter_r,
	XalanUnicode::charLetter_u,
	XalanUnicode::charLetter_e,
	0
};


const XalanDOMChar	PrintWriter::s_falseString[] =
{
	XalanUnicode::charLetter_f,
	XalanUnicode::charLetter_a,
	XalanUnicode::charLetter_l,
	XalanUnicode::charLetter_s,
	XalanUnicode::charLetter_e,
	0
};


const XalanDOMChar	PrintWriter::s_newlineString[] =
{
	XalanUnicode::charLF,
	0
};



PrintWriter::PrintWriter( bool	fAutoFlush, MemoryManagerType& theManager) :
	Writer(),
	m_fAutoFlush(fAutoFlush),
    m_memoryManager(theManager)
{
}



PrintWriter::~PrintWriter()
{
}



XALAN_CPP_NAMESPACE_END
