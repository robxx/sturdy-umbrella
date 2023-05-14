/**************************************************************************
* ADOBE SYSTEMS INCORPORATED
* Copyright 2008 Adobe Systems Incorporated
* All Rights Reserved
*
* NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the
* terms of the Adobe license agreement accompanying it.  If you have received this file from a
* source other than Adobe, then your use, modification, or distribution of it requires the prior
* written permission of Adobe.
**************************************************************************/

#define WINDOWS_IGNORE_PACKING_MISMATCH

#include "BasicExternalObject.h"
#include "SoSharedLibDefs.h"
#include <string>

#include <curl/curl.h>


/**
* \brief To allow string manipulation
*/
using namespace std;

/** This is the version number, and can be modified by setVersion(). */
static long libraryVersionNumber = 1;

/**
	String string that contains the signatures of every ESFunction defined here,
	used to support the JavaScript reflection interface.
	Note that this is a single comma-separated values string, concatenated
	by the compiler.
*/
static char* signatures = "httpPost_sss"; // ?

#if MAC
#define unused(a) (void*) a ;
#else
void* unused(void* x) { return x; };
#endif

/**
* \brief Returns the version number of the library
*
* ExtendScript publishes this number as the version property of the object
* created by new ExternalObject.  Used by the direct interface.
*/
extern "C" ESCURLLIB long ESGetVersion()
{
	return libraryVersionNumber;
}

/**
* \brief Initialize the library and return function signatures.
*
* These signatures have no effect on the arguments that can be passed to the functions.
* They are used by JavaScript to cast the arguments, and to populate the
* reflection interface.
*/
extern "C" ESCURLLIB char* ESInitialize(TaggedData * argv, long argc)
{
	unused(&argv);
	unused(&argc);

	curl_global_init(CURL_GLOBAL_ALL);

	return signatures;
}

/**
* \brief Terminate the library.
*
* Does any necessary clean up that is needed.
*/
extern "C" ESCURLLIB void ESTerminate()
{
	/* we are done with libcurl, so clean it up */
	curl_global_cleanup();
}

/**
* \brief Free any string memory which has been returned as function result.
* JavaScipt calls this function to release the memory associated with the string.
* Used for the direct interface.
*
* \param *p Pointer to the string
*/
extern "C" ESCURLLIB void ESFreeMem(void* p)
{
	if (p)
		free(p);
}


/**







*/
struct MemoryStruct {
	char* memory;
	size_t size;
};
static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct* mem = (struct MemoryStruct*)userp;

	mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory == NULL) {
		/* out of memory */
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}
/**







*/
extern "C" ESCURLLIB long httpPost(TaggedData * argv, long argc, TaggedData * retval) {
	// Accept 2 arguments
	if (argc != 3)
	{
		return kESErrBadArgumentList;
	}

	// The arguments must be a string
	if (argv[0].type != kTypeString) // url
	{
		return kESErrBadArgumentList;
	}
	if (argv[1].type != kTypeString) // json request data
	{
		return kESErrBadArgumentList;
	}
	if (argv[2].type != kTypeString) // auth header
	{
		return kESErrBadArgumentList;
	}

	CURLcode ret;
	CURL* hnd;
	struct curl_slist* slist1;

	slist1 = NULL;
	slist1 = curl_slist_append(slist1, argv[2].data.string);
	slist1 = curl_slist_append(slist1, "Content-Type: application/json");

	struct MemoryStruct chunk;

	chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
	chunk.size = 0;    /* no data at this point */

	

	/* init the curl session */
	hnd = curl_easy_init();


	// argv[0].data.string = the string passed in from the script
	curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
	curl_easy_setopt(hnd, CURLOPT_URL, argv[0].data.string);
	curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
	// post data
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, argv[1].data.string);
	// content length
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)strlen(argv[1].data.string));
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
	curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.81.0");
	curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
	curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
	curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
	curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);

	/* send all data to this function  */
	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void*)&chunk);


	ret = curl_easy_perform(hnd);

	/* check for errors */
	if (ret != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
			curl_easy_strerror(ret));
	}
	else {
		/*
		 * Now, our chunk.memory points to a memory block that is chunk.size
		 * bytes big and contains the remote file.
		 *
		 * Do something nice with it!
		 */

		printf("%lu bytes retrieved\n", (long)chunk.size);
	}



	/* cleanup curl stuff */
	curl_easy_cleanup(hnd);
	hnd = NULL;
	curl_slist_free_all(slist1);
	slist1 = NULL;

	
	// The returned value type
	retval->type = kTypeString;

	retval->data.string = chunk.memory;


	return kESErrOK;
}
