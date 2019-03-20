#include <emscripten.h>
#include <emscripten/bind.h>

#include <SDL/SDL.h>
#include <SDL/SDL_video.h>
#include <SDL/SDL_render.h>
#include <SDL/SDL_rect.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_mutex.h>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/error.h"
};

#include <string>

const unsigned int PER_DATA = 32 * 1024;

class WebPlayer
{
public:
	WebPlayer();
	~WebPlayer();

	bool InitPlayer();

	bool SetStreamTypeProbeParams(int probesize , int max_analyze_duration );

	bool CreateWindow(int x, int y, int w, int h);

	bool Play();

	bool InputData(const char* pData,unsigned int size);

	void Stop();

	void DestroyWindow();

private:
	static int fill_iobuffer(void * opaque, uint8_t *buf, int bufsize);
	static int sdl_thread_handle_initial_data(void *opaque);

	int readBuffer(uint8_t *buf, int bufsize);

	enum PalyStatus
	{
		Status_No_Init = -2,
		Status_No_Play,
		Status_No_StreamType,
		Status_Playing
	};

private:
	void procInitailDataThread();
	bool procData();

private:
	SDL_Window* 	m_screen;
	SDL_Rect 		m_rect;
	SDL_Renderer* 	m_renderer;
	SDL_Texture* 	m_texture;
	AVFormatContext* m_pfmt_ctx;
	AVIOContext*	m_pio_ctx;
	SDL_Thread*     m_thread;
	AVCodecParameters* m_pcodec_par;
	AVCodec*		m_pcodec;
	AVCodecContext* m_pcodec_ctx;
	AVFrame*        m_pfrm_raw;        // 帧，由包解码得到原始帧
	AVFrame*        m_pfrm_yuv;        // 帧，由原始帧色彩转换得到
	AVPacket*       m_ppacket;         // 包，从流中读出的一段数据
	uint8_t*		m_pYUVdata;
	SwsContext*		m_psws_ctx;
	int 			m_iInitStatus;
	unsigned int	m_video_idx;

	std::string m_buffer;
	SDL_cond *	m_cond;
	SDL_mutex *	m_mutex;
};

WebPlayer::WebPlayer()
	: m_screen(NULL)
	, m_rect{0}
	, m_renderer(NULL)
	, m_texture(NULL)
	, m_pfmt_ctx(NULL)
	, m_pio_ctx(NULL)
	, m_thread(NULL)
	, m_pcodec_par(NULL)
	, m_pcodec(NULL)
	, m_pcodec_ctx(NULL)
	, m_pfrm_raw(NULL)
	, m_pfrm_yuv(NULL)
	, m_ppacket(NULL)
	, m_pYUVdata(NULL)
	, m_psws_ctx(NULL)
	, m_iInitStatus(Status_No_Init)
	, m_video_idx(-1)
{
	m_cond = SDL_CreateCond();
	m_mutex = SDL_CreateMutex();
}

WebPlayer::~WebPlayer()
{
	DestroyWindow();

	if (m_pio_ctx)
	{
		avformat_close_input(&m_pfmt_ctx);

		av_freep(&m_pio_ctx->buffer);
		avio_context_free(&m_pio_ctx);
	}

	if (m_pfmt_ctx)
	{
		avformat_free_context(m_pfmt_ctx);
		m_pfmt_ctx = NULL;
	}

	SDL_DestroyCond(m_cond);
	SDL_DestroyMutex(m_mutex);
}

int WebPlayer::fill_iobuffer(void * opaque, uint8_t *buf, int bufsize)
{
	if (bufsize <= 0)
	{
		return -1;
	}
	WebPlayer* pThis = (WebPlayer*)opaque;
	return pThis->readBuffer(buf,bufsize);
}

int WebPlayer::sdl_thread_handle_initial_data(void *opaque)
{
	WebPlayer* pThis = (WebPlayer*)opaque;
	pThis->procInitailDataThread();
}

int WebPlayer::readBuffer(uint8_t *buf, int bufsize)
{
	int n = bufsize;
	if (Status_No_StreamType == m_iInitStatus)
	{
		SDL_LockMutex(m_mutex);
		while (m_buffer.empty())
		{
			SDL_CondWait(m_cond, m_mutex);
		}
		n = m_buffer.length();
		n = n > bufsize ? bufsize : n;

		memcpy(buf, m_buffer.data(), n);
		//s_buffer.clear();
		m_buffer = m_buffer.substr(n);
		SDL_UnlockMutex(m_mutex);
	}
	else if (Status_Playing == m_iInitStatus)
	{
		n = m_buffer.length();
		if (n > bufsize)
		{
			n = bufsize;
			memcpy(buf, m_buffer.data(), n);
			m_buffer = m_buffer.substr(n);
		}
		else
		{
			memcpy(buf, m_buffer.data(), n);
			m_buffer.clear();
		}
	}

	return n;
}

bool WebPlayer::InitPlayer()
{
	if (m_iInitStatus != Status_No_Init)
	{
		return true;
	}

	bool bRet = false;

	do
	{
		m_pfmt_ctx = avformat_alloc_context();
		if (!m_pfmt_ctx)
		{
			break;
		}

		uint8_t* iobuffer = (unsigned char *)av_malloc(PER_DATA);
		m_pio_ctx = avio_alloc_context(iobuffer, PER_DATA, 0, this, &WebPlayer::fill_iobuffer, NULL, NULL);
		if (!m_pio_ctx)
		{
			break;
		}

		m_pfmt_ctx->pb = m_pio_ctx;

		int ret = avformat_open_input(&m_pfmt_ctx, "nothing", NULL, NULL);
		if (ret != 0)
		{
			printf("avformat_open_input() failed %d\n", ret);
			break;
		}		

		bRet = true;
	} while (0);

	if (false == bRet)
	{
		if (m_pio_ctx)
		{			
			avformat_close_input(&m_pfmt_ctx);
			
			av_freep(&m_pio_ctx->buffer);
			avio_context_free(&m_pio_ctx);
		}

		if (m_pfmt_ctx)
		{
			avformat_free_context(m_pfmt_ctx);
			m_pfmt_ctx = NULL;
		}
	}
	else
	{
		m_iInitStatus = Status_No_Play;
	}

	return bRet;	
}

bool WebPlayer::SetStreamTypeProbeParams(int probesize, int max_analyze_duration)
{
	if (m_iInitStatus == Status_No_Init)
	{
		return false;
	}

	if (probesize)
	{
		m_pfmt_ctx->probesize = probesize;
	}

	if (max_analyze_duration)
	{
		m_pfmt_ctx->max_analyze_duration = max_analyze_duration;
	}

	return true;
}

bool WebPlayer::CreateWindow(int x, int y, int w, int h)
{
	bool bRet = false;

	do
	{
		m_screen = SDL_CreateWindow("", x, y, w, h, SDL_WINDOW_OPENGL);
		if (NULL == m_screen)
		{
			printf("SDL_CreateRenderer() failed: %s\n", SDL_GetError());
			break;
		}

		//SDL_Renderer：渲染器
		m_renderer = SDL_CreateRenderer(m_screen, -1, 0);
		if (NULL == m_renderer)
		{
			printf("SDL_CreateRenderer() failed: %s\n", SDL_GetError());
			break;
		}

		//一个SDL_Texture对应一帧YUV数据，同SDL 1.x中的SDL_Overlay
		//此处第2个参数使用的是SDL中的像素格式
		m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, w, h);
		if (NULL == m_texture)
		{
			printf("SDL_CreateTexture() failed: %s\n", SDL_GetError());
			break;
		}

		m_rect.x = 0;
		m_rect.y = 0;
		m_rect.w = w;
		m_rect.h = h;

		bRet = true;

	} while (0);

	if (false == bRet)
	{
		if (m_texture)
		{
			SDL_DestroyTexture(m_texture);
			m_texture = NULL;
		}

		if (m_renderer)
		{
			SDL_DestroyRenderer(m_renderer);
			m_renderer = NULL;
		}

		if (m_screen)
		{
			SDL_DestroyWindow(m_screen);
			m_screen = NULL;
		}
	}

	return bRet;
}

bool WebPlayer::Play()
{
	if (m_iInitStatus == Status_No_Init)
	{
		return false;
	}

	if (m_iInitStatus != Status_No_Play)
	{
		return true;
	}

	m_iInitStatus = Status_No_StreamType;

	if (m_thread)
	{
		SDL_WaitThread(m_thread,NULL);
	}

	m_thread = SDL_CreateThread(&WebPlayer::sdl_thread_handle_initial_data, (void *)this);
 	if (m_thread == NULL)
 	{
		m_iInitStatus = Status_No_Play;
 		printf("SDL_CreateThread() failed: %s\n", SDL_GetError());		
		return false;
 	}
	return true;
}

bool WebPlayer::InputData(const char* pData, unsigned int size)
{
	if (Status_No_StreamType == m_iInitStatus)
	{
		SDL_LockMutex(m_mutex);
		m_buffer.append(pData, size);
		SDL_CondSignal(m_cond);
		SDL_UnlockMutex(m_mutex);

		return true;
	}
	else if(Status_Playing == m_iInitStatus)
	{
		m_buffer.append(pData, size);
		return procData();
	}

	return false;
}

void WebPlayer::Stop()
{
	if (m_iInitStatus == Status_No_Init
		|| m_iInitStatus == Status_No_Play)
	{
		return;
	}

	if (m_thread)
	{
		SDL_WaitThread(m_thread,NULL);
		m_thread = NULL;
	}

	m_iInitStatus = Status_No_Play;

	if (m_psws_ctx)
	{
		av_packet_free(&m_ppacket);
		sws_freeContext(m_psws_ctx);
		m_psws_ctx = NULL;
	}

	if (m_pYUVdata)
	{
		av_free(m_pYUVdata);
		m_pYUVdata = NULL;
	}

	if (m_pfrm_yuv)
	{
		av_frame_free(&m_pfrm_yuv);
	}

	if (m_pfrm_raw)
	{
		av_frame_free(&m_pfrm_raw);
	}

	if (m_pcodec_ctx)
	{
		avcodec_free_context(&m_pcodec_ctx);
	}

	if (m_pcodec)
	{
		m_pcodec = NULL;
	}	
}

void WebPlayer::DestroyWindow()
{
	Stop();

	if (m_texture)
	{
		SDL_DestroyTexture(m_texture);
		m_texture = NULL;
	}

	if (m_renderer)
	{
		SDL_DestroyRenderer(m_renderer);
		m_renderer = NULL;
	}

	if (m_screen)
	{
		SDL_DestroyWindow(m_screen);
		m_screen = NULL;
	}
}

void WebPlayer::procInitailDataThread()
{
	bool bRet = false;
	do 
	{
		//搜索流信息：读取一段数据，尝试解码，将取到的流信息填入pFormatCtx->streams
		// m_pfmt_ctx->streams是一个指针数组，数组大小是pFormatCtx->nb_streams
		int ret = avformat_find_stream_info(m_pfmt_ctx, NULL);
		if (ret < 0)
		{
			printf("avformat_find_stream_info() failed %d\n", ret);
			break;
		}

		// 查找第一个视频流
		for (unsigned int i = 0; i < m_pfmt_ctx->nb_streams; i++)
		{
			if (m_pfmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				m_video_idx = i;
				break;
			}
		}

		if (m_video_idx == -1)
		{
			printf("Cann't find a video stream\n");
			break;
		}

		// 为视频流构建解码器AVCodecContext
		// 获取解码器参数AVCodecParameters
		m_pcodec_par = m_pfmt_ctx->streams[m_video_idx]->codecpar;

		// 获取解码器
		m_pcodec = avcodec_find_decoder(m_pcodec_par->codec_id);
		if (m_pcodec == NULL)
		{
			printf("Cann't find codec!\n");
			break;
		}

		// 构建解码器AVCodecContext
		// p_codec_ctx初始化：分配结构体，使用p_codec初始化相应成员为默认值
		m_pcodec_ctx = avcodec_alloc_context3(m_pcodec);
		if (m_pcodec_ctx == NULL)
		{
			printf("avcodec_alloc_context3() failed %d\n", ret);
			break;
		}
		// p_codec_ctx初始化：p_codec_par ==> p_codec_ctx，初始化相应成员
		ret = avcodec_parameters_to_context(m_pcodec_ctx, m_pcodec_par);
		if (ret < 0)
		{
			printf("avcodec_parameters_to_context() failed %d\n", ret);
			break;
		}
		// p_codec_ctx初始化：使用p_codec初始化p_codec_ctx，初始化完成
		ret = avcodec_open2(m_pcodec_ctx, m_pcodec, NULL);
		if (ret < 0)
		{
			printf("avcodec_open2() failed %d\n", ret);
			break;
		}

		// 分配AVFrame
		// 分配AVFrame结构，注意并不分配data buffer(即AVFrame.*data[])
		m_pfrm_raw = av_frame_alloc();
		if (m_pfrm_raw == NULL)
		{
			printf("av_frame_alloc() for p_frm_raw failed\n");
			break;
		}

		m_pfrm_yuv = av_frame_alloc();
		if (m_pfrm_yuv == NULL)
		{
			printf("av_frame_alloc() for p_frm_yuv failed\n");
			break;
		}

		// 为AVFrame.*data[]手工分配缓冲区，用于存储sws_scale()中目的帧视频数据
		// m_pfrm_raw的data_buffer由av_read_frame()分配，因此不需手工分配
		// m_pfrm_yuv的data_buffer无处分配，因此在此处手工分配
		int buf_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
			m_rect.w,
			m_rect.h,
			1
		);
		// buffer将作为p_frm_yuv的视频数据缓冲区
		m_pYUVdata = (uint8_t *)av_malloc(buf_size);
		if (m_pYUVdata == NULL)
		{
			printf("av_malloc() for buffer failed\n");
			break;
		}
		// 使用给定参数设定m_pfrm_yuv->data和m_pfrm_yuv->linesize
		ret = av_image_fill_arrays(m_pfrm_yuv->data,     // dst data[]
			m_pfrm_yuv->linesize,	// dst linesize[]
			m_pYUVdata,				// src buffer
			AV_PIX_FMT_YUV420P,		// pixel format
			m_rect.w,				// width
			m_rect.h,				// height
			1						// align
		);
		if (ret < 0)
		{
			printf("av_image_fill_arrays() failed %d\n", ret);
			break;
		}

		//	   初始化SWS context，用于后续图像转换
		//     此处第6个参数使用的是FFmpeg中的像素格式
		//     FFmpeg中的像素格式AV_PIX_FMT_YUV420P对应SDL中的像素格式SDL_PIXELFORMAT_IYUV
		//     如果解码后得到图像的不被SDL支持，不进行图像转换的话，SDL是无法正常显示图像的
		//     如果解码后得到图像的能被SDL支持，则不必进行图像转换
		//     这里为了编码简便，统一转换为SDL支持的格式AV_PIX_FMT_YUV420P==>SDL_PIXELFORMAT_IYUV
		m_psws_ctx = sws_getContext(m_pcodec_ctx->width,    // src width
			m_pcodec_ctx->height,   // src height
			m_pcodec_ctx->pix_fmt,  // src format
			m_rect.w,				// dst width
			m_rect.h,				// dst height
			AV_PIX_FMT_YUV420P,    // dst format
			SWS_FAST_BILINEAR,     // flags
			NULL,                  // src filter
			NULL,                  // dst filter
			NULL                   // param
		);
		if (m_psws_ctx == NULL)
		{
			break;
		}

		m_ppacket = (AVPacket *)av_malloc(sizeof(AVPacket));
		if (m_ppacket == NULL)
		{
			printf("SDL_CreateThread() failed: %s\n", SDL_GetError());
			break;
		}

		bRet = true;
	} while (0);

	if (false == bRet)
	{
		if (m_psws_ctx)
		{
			av_packet_free(&m_ppacket);
			sws_freeContext(m_psws_ctx);
			m_psws_ctx = NULL;
		}

		if (m_pYUVdata)
		{
			av_free(m_pYUVdata);
			m_pYUVdata = NULL;
		}

		if (m_pfrm_yuv)
		{
			av_frame_free(&m_pfrm_yuv);
		}

		if (m_pfrm_raw)
		{
			av_frame_free(&m_pfrm_raw);
		}

		if (m_pcodec_ctx)
		{
			avcodec_free_context(&m_pcodec_ctx);
		}

		if (m_pcodec)
		{
			m_pcodec = NULL;
		}

		m_iInitStatus = Status_No_Play;
	}
	else
	{
		m_iInitStatus = Status_Playing;
	}
}

bool WebPlayer::procData()
{
	bool bRet = false;

	do 
	{
		//     packet可能是视频帧、音频帧或其他数据，解码器只会解码视频帧或音频帧，非音视频数据并不会被
		//     扔掉、从而能向解码器提供尽可能多的信息
		//     对于视频来说，一个packet只包含一个frame
		//     对于音频来说，若是帧长固定的格式则一个packet可包含整数个frame，
		//                   若是帧长可变的格式则一个packet只包含一个frame
		while (av_read_frame(m_pfmt_ctx, m_ppacket) == 0) //fill_iobuffer 回调会被调用
		{
			if (m_ppacket->stream_index == m_video_idx)  // 直到取到一帧视频帧
			{
				break;
			}
		}

		// 视频解码：packet ==> frame
		// 向解码器喂数据，一个packet可能是一个视频帧或多个音频帧，此处音频帧已被上一句滤掉
		int ret = avcodec_send_packet(m_pcodec_ctx, m_ppacket);
		if (ret != 0)
		{
			printf("avcodec_send_packet() failed %d\n", ret);
			break;
		}
		// 接收解码器输出的数据，此处只处理视频帧，每次接收一个packet，将之解码得到一个frame
		ret = avcodec_receive_frame(m_pcodec_ctx, m_pfrm_raw);
		if (ret != 0)
		{
			if (ret == AVERROR_EOF)
			{
				printf("avcodec_receive_frame(): the decoder has been fully flushed\n");
			}
			else if (ret == AVERROR(EAGAIN))
			{
// 				printf("avcodec_receive_frame(): output is not available in this state - "
// 					"user must try to send new input\n");
				bRet = true;				
			}
			else if (ret == AVERROR(EINVAL))
			{
				printf("avcodec_receive_frame(): codec not opened, or it is an encoder\n");
			}
			else
			{
				printf("avcodec_receive_frame(): legitimate decoding errors\n");
			}
			break;
		}

		// 图像转换：p_frm_raw->data ==> p_frm_yuv->data
		// 将源图像中一片连续的区域经过处理后更新到目标图像对应区域，处理的图像区域必须逐行连续
		// plane: 如YUV有Y、U、V三个plane，RGB有R、G、B三个plane
		// slice: 图像中一片连续的行，必须是连续的，顺序由顶部到底部或由底部到顶部
		// stride/pitch: 一行图像所占的字节数，Stride=BytesPerPixel*Width+Padding，注意对齐
		// AVFrame.*data[]: 每个数组元素指向对应plane
		// AVFrame.linesize[]: 每个数组元素表示对应plane中一行图像所占的字节数
		sws_scale(m_psws_ctx,                         // sws context
			(const uint8_t *const *)m_pfrm_raw->data, // src slice
			m_pfrm_raw->linesize,                     // src stride
			0,                                        // src slice y
			m_pcodec_ctx->height,                     // src slice height
			m_pfrm_yuv->data,                         // dst planes
			m_pfrm_yuv->linesize                      // dst strides
		);

		// 使用新的YUV像素数据更新SDL_Rect
		SDL_UpdateTexture(m_texture,     // sdl texture
			&m_rect,                     // sdl rect
			m_pfrm_yuv->data[0],         // y plane
			m_pfrm_yuv->linesize[0]      // y pitch 			
		);

		// 使用特定颜色清空当前渲染目标
		SDL_RenderClear(m_renderer);
		// 使用部分图像数据(texture)更新当前渲染目标
		SDL_RenderCopy(m_renderer,          // sdl renderer
			m_texture,                      // sdl texture
			NULL,                           // src rect, if NULL copy texture
			&m_rect                         // dst rect
		);

		// 执行渲染，更新屏幕显示
		SDL_RenderPresent(m_renderer);

		av_packet_unref(m_ppacket);

		bRet = true;
	} while (0);

	return bRet;
}


// Binding code
EMSCRIPTEN_BINDINGS(HikWebPlayer) {
	class_<WebPlayer>("WebPlayer")
		.constructor<>()
		.function("InitPlayer", &WebPlayer::InitPlayer)
		.function("SetStreamTypeProbeParams", &WebPlayer::SetStreamTypeProbeParams)
		.function("CreateWindow", &WebPlayer::CreateWindow)
		.function("Play", &WebPlayer::Play)
		.function("InputData", &WebPlayer::InputData)
		.function("Stop", &WebPlayer::Stop)
		.function("DestroyWindow", &WebPlayer::DestroyWindow)
		;
}


// The "main loop" function.
void one_iter()
{
	// process input
	// render to screen
	SDL_Event e;
	if (1 == SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT)
		{
			SDL_Quit();
		}
	}	
}

int main()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER))
	{
		printf("SDL_Init() failed: %s\n", SDL_GetError());
		return -1;
	}

	// void emscripten_set_main_loop(em_callback_func func, int fps, int simulate_infinite_loop);
	emscripten_set_main_loop(one_iter, 60, 1);	

	return 0;
}

