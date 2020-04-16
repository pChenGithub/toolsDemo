/*
* 从海康摄像头获取rtsp流，保存到文件mp4
*/

#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag)
{
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    printf("%s: pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
           tag,
           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
           pkt->stream_index);
}

int main(int argc, char **argv)
{
    AVOutputFormat *ofmt = NULL;
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
    AVPacket pkt;
    const char *in_filename, *out_filename;
    int ret, i;
    int stream_index = 0;
    int *stream_mapping = NULL;
    int stream_mapping_size = 0;
	time_t t;
	time_t new;
	struct tm *info;
	char savefile[32];
	int x;

    if (argc < 3) {
        printf("usage: %s input output\n"
               "API example program to remux a media file with libavformat and libavcodec.\n"
               "The output format is guessed according to the file extension.\n"
               "\n", argv[0]);
        return 1;
    }

    in_filename  = argv[1];
    out_filename = argv[2];

    av_register_all();

	// 获取输入媒体格式上下文
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        fprintf(stderr, "Could not open input file '%s'", in_filename);
        goto end;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        fprintf(stderr, "Failed to retrieve input stream information");
        goto end;
    }

	// 输出输入流信息
    av_dump_format(ifmt_ctx, 0, in_filename, 0);


	// 获取输出流媒体格式上下文
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
    if (!ofmt_ctx) {
        fprintf(stderr, "Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }

	// 获取输入流的媒体轨数量 视频/音频/字幕 等
    stream_mapping_size = ifmt_ctx->nb_streams;
    stream_mapping = av_mallocz_array(stream_mapping_size, sizeof(*stream_mapping));
    if (!stream_mapping) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

	// 获取输出流的封装格式
    ofmt = ofmt_ctx->oformat;

	// 查看输入媒体有多少条流轨，为每条轨在输出流中创建相应的轨，并且设置编码格式和参数
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream *out_stream;
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVCodecParameters *in_codecpar = in_stream->codecpar;

		// 过滤，这里只需要了 音频/视频/字幕
        if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
            stream_mapping[i] = -1;
            continue;
        }

        stream_mapping[i] = stream_index++;

		// 创建一条轨
        out_stream = avformat_new_stream(ofmt_ctx, NULL);
        if (!out_stream) {
            fprintf(stderr, "Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            goto end;
        }

		// 设置编码参数，这里直接把输入流的相应轨的编码参数拷贝，因为没有编码的修改，也就是没有转码
        ret = avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
        if (ret < 0) {
            fprintf(stderr, "Failed to copy codec parameters\n");
            goto end;
        }
        out_stream->codecpar->codec_tag = 0;
    }

	// 输出输出流的信息
    av_dump_format(ofmt_ctx, 0, out_filename, 1);

	int64_t start_pts = 0;
	int64_t start_dts = 0;
	int64_t start_pts_save = 0;
	int64_t start_dts_save = 0;

	//开始处理流，按时间保存文件
	time(&t); // 获取时间戳
	info = localtime(&t);
	printf("time: %s \n", asctime(info));
	sprintf(savefile, "save_%02d_%02d_%02d.mp4", info->tm_hour, info->tm_min, info->tm_sec);
	i = 0;
	do {
		if (!(ofmt->flags & AVFMT_NOFILE)) {
			// 设置输出流的关联文件，
			//ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
			ret = avio_open(&ofmt_ctx->pb, savefile, AVIO_FLAG_WRITE);
			if (ret < 0) {
				fprintf(stderr, "Could not open output file '%s'", out_filename);
				goto end;
			}
		}

		// 写文件头
		ret = avformat_write_header(ofmt_ctx, NULL);
		if (ret < 0) {
			fprintf(stderr, "Error occurred when opening output file\n");
			goto end;
		}

		// 循环获取输入流帧，写入到输出流
		while (1) {
			AVStream *in_stream, *out_stream;

			// 获取一帧
			ret = av_read_frame(ifmt_ctx, &pkt);
			if (ret < 0)
				break;

			in_stream  = ifmt_ctx->streams[pkt.stream_index];
			if (pkt.stream_index >= stream_mapping_size ||
					stream_mapping[pkt.stream_index] < 0) {
				av_packet_unref(&pkt);
				continue;
			}


			// 这边丢帧的话会发生视音不正常，花屏/速度变快
			//if (i%4) {
			//	i++;
			//	continue ;
			//}

			//i++ ;

			pkt.stream_index = stream_mapping[pkt.stream_index];
			out_stream = ofmt_ctx->streams[pkt.stream_index];
//			log_packet(ifmt_ctx, &pkt, "in");

			start_pts_save = pkt.pts;
			start_dts_save = pkt.dts;
			pkt.pts = pkt.pts - start_pts;
			pkt.dts = pkt.dts - start_dts;
			/* copy packet */
			pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
			pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
			pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
			pkt.pos = -1;
			log_packet(ofmt_ctx, &pkt, "out");

			// 写入一帧
			ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
			if (ret < 0) {
				fprintf(stderr, "Error muxing packet\n");
				break;
			}
			//i++;
			av_packet_unref(&pkt);

			//	if (i>1000)
			//		break;
			time(&new);
			x = new/60 - t/60;
			//printf("x: %d \n", x);
			if (x) {
				printf("next file \n");
				start_pts = start_pts_save;
				start_dts = start_dts_save;
				out_stream->start_time = 0;
				out_stream->duration = 0;
				out_stream->first_dts = start_dts;
				out_stream->cur_dts = 0;
				out_stream->last_IP_pts = 0;
				break;
			}

		}
		// 写文件尾
		av_write_trailer(ofmt_ctx);
		avio_closep(&ofmt_ctx->pb);

		t = new;
		info = localtime(&t);
		printf("time: %s \n", asctime(info));
		sprintf(savefile, "save_%02d_%02d_%02d.mp4", info->tm_hour, info->tm_min, info->tm_sec);
		//sprintf(savefile, "%s.mp4", asctime(info));

		i++;
		if (i>2)
			break;
	} while(1) ;

end:

    avformat_close_input(&ifmt_ctx);

    /* close output */
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
        avio_closep(&ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);

    av_freep(&stream_mapping);

    if (ret < 0 && ret != AVERROR_EOF) {
        fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
        return 1;
    }

    return 0;
}
