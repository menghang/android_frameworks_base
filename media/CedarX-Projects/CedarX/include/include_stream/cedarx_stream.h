#ifndef CEDARX_STREAM_H_
#define CEDARX_STREAM_H_

//#define _FILE_OFFSET_BITS 64
//#define __USE_FILE_OFFSET64
//#define __USE_LARGEFILE64
//#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include <unistd.h>

#include <CDX_Common.h>
#include <include_base/tsemaphore.h>

#if (_FILE_OFFSET_BITS==64)
#define cdx_off_t long long
#else
#define cdx_off_t long long
#endif

/*   stream.h */
#define STREAM_BUFFER_SIZE 2048
#define VCD_SECTOR_SIZE 2352
#define VCD_SECTOR_OFFS 24
#define VCD_SECTOR_DATA 2324

#define DRAM_BUF_LEN  (1024*256)
typedef struct cdx_data_buf{
	unsigned char * buf;
	unsigned int    buf_len;
	unsigned char * buf_pos;
	unsigned int 	data_len;
}cdx_data_buf_t;

typedef struct cdx_stream_info {
  const char *info;
  const char *name;
  const char *comment;

  int 					quitFlag;
  cdx_sem_t            	sem_data_ready;
  reqdata_from_dram  	request_data;
  cdx_data_buf_t    	data_buf;
  int					isReqData;
  CedarXDataSourceDesc data_src_desc;

  FILE  *file_handle;
#ifdef __OS_ANDROID
  int   file_descriptor;
#endif

  int  (*seek)(struct cdx_stream_info *stream, cdx_off_t offset, int whence);
  cdx_off_t (*tell)(struct cdx_stream_info *stream);
  int  (*read)(void *ptr, size_t size, size_t nmemb, struct cdx_stream_info *stream);
  int  (*write)(const void *ptr, size_t size, size_t nmemb, struct cdx_stream_info *stream);
  long long (*getsize)(struct cdx_stream_info *stream);

  //below two function used for m3u/ts
  long long (*seek_to_time)(struct cdx_stream_info *stream, long long us);
  long long (*get_total_duration)(struct cdx_stream_info *stream);

  void (*reset_stream)(struct cdx_stream_info *stream);
  int (*control_stream)(struct cdx_stream_info * stream, void *arg, int cmd);
} cdx_stream_info_t;

extern struct cdx_stream_info *create_stream_handle(CedarXDataSourceDesc *datasource_desc);
extern void destory_stream_handle(struct cdx_stream_info *stm_info);

static inline int cdx_seek(struct cdx_stream_info *stream, cdx_off_t offset, int whence)
{
	return stream->seek(stream, offset, whence);
}

static inline cdx_off_t cdx_tell(struct cdx_stream_info *stream)
{
	return stream->tell(stream);
}

static inline int cdx_read(void *ptr, size_t size, size_t nmemb, struct cdx_stream_info *stream)
{
	return stream->read(ptr, size, nmemb,stream);
}

static inline int cdx_write(const void *ptr, size_t size, size_t nmemb, struct cdx_stream_info *stream)
{
	return stream->write(ptr, size, nmemb,stream);
}

#endif /* CEDAR_DEMUX_H_ */
