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
#include "xalanc/XSLT/AVTPartSimple.hpp"



#include <xalanc/PlatformSupport/DOMStringHelper.hpp>



#include "xalanc/XSLT/StylesheetConstructionContext.hpp"



XALAN_CPP_NAMESPACE_BEGIN



/**
 * Simple string part of a complex AVT.
 */
AVTPartSimple::AVTPartSimple(
			StylesheetConstructionContext&	constructionContext,
			const XalanDOMChar*				val,
			XalanDOMString::size_type		len) :
	AVTPart(),
	m_val(constructionContext.allocateXalanDOMCharVector(val, len, false)),
	m_len(len)
{
}



void
AVTPartSimple::evaluate(
			XalanDOMString&			buf,
			XalanNode*				/* contextNode */,
			const PrefixResolver&	/* prefixResolver */,
			XPathExecutionContext&	/* executionContext */) const

{
	append(buf, m_val, m_len);
}



void
AVTPartSimple::evaluate(
			XalanDOMString&			buf,
			const PrefixResolver&	/* prefixResolver */,
			XPathExecutionContext&	/* executionContext */) const

{
	append(buf, m_val, m_len);
}



XALAN_CPP_NAMESPACE_END
