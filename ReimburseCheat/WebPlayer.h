#include <SDL/SDL_rect.h>
#include <string>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
struct AVFormatContext;
struct AVIOContext;
struct AVCodecParameters;
struct AVCodec;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
struct SwsContext;

class WebPlayer
{
public:
	WebPlayer();
	~WebPlayer();

	bool InitPlayer(int x, int y, int w, int h);

	bool SetStreamTypeProbeParams(int probesize , int max_analyze_duration );

	bool CreateWindow();

	bool Play();

	bool InputData(const std::string& data);

	void Stop();

	void DestroyWindow();

private:
	friend int fill_iobuffer(void * opaque, uint8_t *buf, int bufsize);
	int readBuffer(uint8_t *buf, int bufsize);

private:
	void procInitailData();
	bool procData();

private:
	SDL_Window* 	m_screen;
	SDL_Rect 		m_rect;
	SDL_Renderer* 	m_renderer;
	SDL_Texture* 	m_texture;
	AVFormatContext* m_pfmt_ctx;
	AVIOContext*	m_pio_ctx;
	AVCodecParameters* m_pcodec_par;
	AVCodec*		m_pcodec;
	AVCodecContext* m_pcodec_ctx;
	AVFrame*        m_pfrm_raw;        // 帧，由包解码得到原始帧
	AVFrame*        m_pfrm_yuv;        // 帧，由原始帧色彩转换得到
	AVPacket*       m_ppacket;         // 包，从流中读出的一段数据
	uint8_t*		m_pYUVdata;
	SwsContext*		m_psws_ctx;
	bool 			m_bPlaying;
	unsigned int	m_video_idx;

	std::string m_buffer;
};
