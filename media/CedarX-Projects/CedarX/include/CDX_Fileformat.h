#ifndef CDX_FILEFORMAT_H_
#define CDX_FILEFORMAT_H_

typedef enum CDX_MEDIA_FILE_FORMAT {
	CDX_MEDIA_FILE_FMT_UNKOWN = 0,
	CDX_MEDIA_FILE_FMT_AVI,
	CDX_MEDIA_FILE_FMT_RM,
	CDX_MEDIA_FILE_FMT_RMVB,
	CDX_MEDIA_FILE_FMT_MP4,
	CDX_MEDIA_FILE_FMT_M4V,
	CDX_MEDIA_FILE_FMT_3GP,
	CDX_MEDIA_FILE_FMT_MOV,
	CDX_MEDIA_FILE_FMT_FLV,
	CDX_MEDIA_FILE_FMT_F4V,
	CDX_MEDIA_FILE_FMT_MPEG,
	CDX_MEDIA_FILE_FMT_MPG,
	CDX_MEDIA_FILE_FMT_VOB,
	CDX_MEDIA_FILE_FMT_DAT,
	CDX_MEDIA_FILE_FMT_MOD,
	CDX_MEDIA_FILE_FMT_PMP,
	CDX_MEDIA_FILE_FMT_WMV,
	CDX_MEDIA_FILE_FMT_ASF,
	CDX_MEDIA_FILE_FMT_MKV,
	CDX_MEDIA_FILE_FMT_PSR,
	CDX_MEDIA_FILE_FMT_RAM,
	CDX_MEDIA_FILE_FMT_SCM,
	CDX_MEDIA_FILE_FMT_OGM,
	CDX_MEDIA_FILE_FMT_M4P,
	CDX_MEDIA_FILE_FMT_M4B,
	CDX_MEDIA_FILE_FMT_TP,
	CDX_MEDIA_FILE_FMT_TPR,
	CDX_MEDIA_FILE_FMT_TS,
	CDX_MEDIA_FILE_FMT_PVA,
	CDX_MEDIA_FILE_FMT_PSS,
	CDX_MEDIA_FILE_FMT_MPE,
	CDX_MEDIA_FILE_FMT_WV,
	CDX_MEDIA_FILE_FMT_M2TS,
	CDX_MEDIA_FILE_FMT_EVO,
	CDX_MEDIA_FILE_FMT_RPM,
	CDX_MEDIA_FILE_FMT_3GPP,
	CDX_MEDIA_FILE_FMT_3G2,
	CDX_MEDIA_FILE_FMT_3GP2,
	CDX_MEDIA_FILE_FMT_QT,
	CDX_MEDIA_FILE_FMT_WMP,
	CDX_MEDIA_FILE_FMT_WM,
	CDX_MEDIA_FILE_FMT_AMV,
	CDX_MEDIA_FILE_FMT_DSM,
	CDX_MEDIA_FILE_FMT_M1V,
	CDX_MEDIA_FILE_FMT_M2V,
	CDX_MEDIA_FILE_FMT_SMK,
	CDX_MEDIA_FILE_FMT_BIK,
	CDX_MEDIA_FILE_FMT_RAT,
	CDX_MEDIA_FILE_FMT_VG2,
	CDX_MEDIA_FILE_FMT_IVF,
	CDX_MEDIA_FILE_FMT_VP6,
	CDX_MEDIA_FILE_FMT_VP7,
	CDX_MEDIA_FILE_FMT_D2V,
	CDX_MEDIA_FILE_FMT_M2P,
	CDX_MEDIA_FILE_FMT_VID,
	CDX_MEDIA_FILE_FMT_PMP2,
	CDX_MEDIA_FILE_FMT_MTS,
	CDX_MEDIA_FILE_FMT_MP3,
	CDX_MEDIA_FILE_FMT_WAV,
	CDX_MEDIA_FILE_FMT_WMA,
	CDX_MEDIA_FILE_FMT_APE,
	CDX_MEDIA_FILE_FMT_FLAC,
	CDX_MEDIA_FILE_FMT_OGG,
	CDX_MEDIA_FILE_FMT_RA,
	CDX_MEDIA_FILE_FMT_MP1,
	CDX_MEDIA_FILE_FMT_MP2,
	CDX_MEDIA_FILE_FMT_AAC,
	CDX_MEDIA_FILE_FMT_AC3,
	CDX_MEDIA_FILE_FMT_DTS,
	CDX_MEDIA_FILE_FMT_AIF,
	CDX_MEDIA_FILE_FMT_AIFF,
	CDX_MEDIA_FILE_FMT_AIFC,
	CDX_MEDIA_FILE_FMT_AMR,
	CDX_MEDIA_FILE_FMT_MAC,
	CDX_MEDIA_FILE_FMT_TTA,
	CDX_MEDIA_FILE_FMT_M4A,
	CDX_MEDIA_FILE_FMT_CDA,
	CDX_MEDIA_FILE_FMT_AU,
	CDX_MEDIA_FILE_FMT_ACC,
	CDX_MEDIA_FILE_FMT_MIDI,
	CDX_MEDIA_FILE_FMT_MID,
	CDX_MEDIA_FILE_FMT_RMI,
	CDX_MEDIA_FILE_FMT_MP5,
	CDX_MEDIA_FILE_FMT_MPA,
	CDX_MEDIA_FILE_FMT_MPGA,
	CDX_MEDIA_FILE_FMT_ACT,
	CDX_MEDIA_FILE_FMT_ATRC,
   	CDX_MEDIA_FILE_FMT_WEBM,

	CDX_MEDIA_FILE_FMT_AUDIO        = (1<<16),
	CDX_MEDIA_FILE_FMT_NETWORK      = (2<<16),
	CDX_MEDIA_FILE_FMT_NETWORK_RTSP = (4<<16),
	CDX_MEDIA_FILE_FMT_IDXSUB       = (8<<16),
	CDX_MEDIA_FILE_FMT_NETWORK_SFT  = (16<<16),
	CDX_MEDIA_FILE_FMT_FD           = (32<<16)
} CDX_MEDIA_FILE_FORMAT;

#endif /* CDX_FILEFORMAT_H_ */
