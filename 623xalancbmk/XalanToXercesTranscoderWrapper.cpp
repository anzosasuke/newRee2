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
#include "xalanc/PlatformSupport/XalanToXercesTranscoderWrapper.hpp"



#include <cassert>



#include <xercesc/util/TransService.hpp>
#include <xercesc/util/XMLException.hpp>

#include <xalanc/Include/XalanMemMgrAutoPtr.hpp>


typedef XERCES_CPP_NAMESPACE_QUALIFIER XMLException		XMLExceptionType;



XALAN_CPP_NAMESPACE_BEGIN



XalanToXercesTranscoderWrapper::XalanToXercesTranscoderWrapper(MemoryManagerType& theManager, XMLTranscoderType&	theTranscoder) :
	XalanOutputTranscoder(theManager),
	m_transcoder(&theTranscoder)
{
}

XalanToXercesTranscoderWrapper*
XalanToXercesTranscoderWrapper::create(MemoryManagerType& theManager, XMLTranscoderType&	theTranscoder)
{
    typedef XalanToXercesTranscoderWrapper ThisType;

    XalanMemMgrAutoPtr<ThisType, false> theGuard( theManager , (ThisType*)theManager.allocate(sizeof(ThisType)));

    ThisType* theResult = theGuard.get();

    new (theResult) ThisType(theManager, theTranscoder);

   theGuard.release();

    return theResult;
}


XalanToXercesTranscoderWrapper::~XalanToXercesTranscoderWrapper()
{
	delete m_transcoder;
}



XalanToXercesTranscoderWrapper::eCode
XalanToXercesTranscoderWrapper::transcode(
			const XalanDOMChar*		theSourceData,
			size_type				theSourceCount,
			XalanXMLByte*			theTarget,
			size_type				theTargetSize,
			size_type&				theSourceCharsTranscoded,
			size_type&				theTargetBytesUsed)
{
	eCode	theCode = XalanTranscodingServices::OK;

	try
	{
		theTargetBytesUsed = m_transcoder->transcodeTo(
			theSourceData,
			theSourceCount,
			theTarget,
			theTargetSize,
			theSourceCharsTranscoded,
			// $$$ ToDo: Eventually, we're going to want to
			// replace this with UnRep_Throw, and let the
			// caller try to recover.
//			XMLTranscoderType::UnRep_Throw);
			XMLTranscoderType::UnRep_RepChar);
	}
	catch(const XMLExceptionType&)
	{
		theSourceCharsTranscoded = 0;
		theTargetBytesUsed = 0;
		theCode = XalanTranscodingServices::InternalFailure;
	}

	return theCode;
}



XalanToXercesTranscoderWrapper::eCode
XalanToXercesTranscoderWrapper::transcode(
			const XalanXMLByte*		theSourceData,
			size_type				theSourceCount,
			XalanDOMChar*			theTarget,
			size_type				theTargetSize,
			size_type&				theSourceCharsTranscoded,
			size_type&				theTargetBytesUsed,
			unsigned char*			theCharSizes)
{
	eCode	theCode = XalanTranscodingServices::OK;

	try
	{
		theTargetBytesUsed =
				m_transcoder->transcodeFrom(
			theSourceData,
			theSourceCount,
			theTarget,
			theTargetSize,
			theSourceCharsTranscoded,
			theCharSizes);
	}
	catch(const XMLExceptionType&)
	{
		theSourceCharsTranscoded = 0;
		theTargetBytesUsed = 0;
		theCode = XalanTranscodingServices::InternalFailure;
	}

	return theCode;
}



bool
XalanToXercesTranscoderWrapper::canTranscodeTo(UnicodeCharType	theChar) const
{
	return m_transcoder->canTranscodeTo(theChar);
}



XALAN_CPP_NAMESPACE_END
