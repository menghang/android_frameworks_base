/*******************************************************************************
--                                                                            --
--                    CedarX Multimedia Framework                             --
--                                                                            --
--          the Multimedia Framework for Linux/Android System                 --
--                                                                            --
--       This software is confidential and proprietary and may be used        --
--        only as expressly authorized by a licensing agreement from          --
--                         Softwinner Products.                               --
--                                                                            --
--                   (C) COPYRIGHT 2011 SOFTWINNER PRODUCTS                   --
--                            ALL RIGHTS RESERVED                             --
--                                                                            --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
--                                                                            --
*******************************************************************************/

#define LOG_NDEBUG 0
#define LOG_TAG "thirdpart_stream"
#include <utils/Log.h>

#include <thirdpart_stream.h>

static int thirdpart_seek_steam(struct cdx_stream_info *stream, cdx_off_t offset, int whence)
{
	LOGV("thirdpart seek");
	return fseeko(stream->file_handle, offset, whence);
}

static cdx_off_t thirdpart_tell_steam(struct cdx_stream_info *stream)
{
	LOGV("thirdpart tell");
	return ftello(stream->file_handle);
}

static int thirdpart_read_steam(void *ptr, size_t size, size_t nmemb, struct cdx_stream_info *stream)
{
	LOGV("thirdpart read");
	return fread(ptr, size, nmemb,stream->file_handle);
}

static int thirdpart_write_steam(const void *ptr, size_t size, size_t nmemb, struct cdx_stream_info *stream)
{
	LOGW("Not Implementation!");
	return 0;
}

static long long thirdpart_get_stream_size(struct cdx_stream_info *stream)
{
	LOGW("Not Implementation!");
	return 0;
}

int thirdpart_create_stream_handle(struct cdx_stream_info *stm_info)
{
	LOGI(">>>> thirdpart stream info:%s",stm_info->data_src_desc.source_url);
	stm_info->file_handle = fopen(stm_info->data_src_desc.source_url,"rb");
	stm_info->seek    = thirdpart_seek_steam ;
	stm_info->tell    = thirdpart_tell_steam ;
	stm_info->read    = thirdpart_read_steam ;
	stm_info->write   = thirdpart_write_steam;
	stm_info->getsize = thirdpart_get_stream_size;

	return 0;
}

void thirdpart_destory_stream_handle(struct cdx_stream_info *stm_info)
{
	LOGI(">>>> thirdpart stream destroy");

	if(stm_info->file_handle){
		fclose(stm_info->file_handle);
		stm_info->file_handle = NULL;
	}
}

