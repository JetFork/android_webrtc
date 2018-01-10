//
//  ViewController.m
//  decTest
//
//  Created by tian.hu on 2017/3/2.
//  Copyright © 2017年 gnet. All rights reserved.
//

#import "ViewController.h"
#import "../../../../inc/ffmpegdemuxdec_api.h"

@interface ViewController ()

@end

@implementation ViewController
#define  INBUF_SIZE 81920

char *audio_dst_filename = NULL;
char *video_dst_filename = NULL;

FILE *audiofp;
FILE *videofp;

void WriteDecodeDate(int gotfame,FrameInfo *frame,void *handle)
{
    switch(gotfame)
    {
        case 0:
            if (videofp != NULL)
            {
                fwrite(frame->Buffer,1,frame->FrameLen,videofp);
                ///printf("read size = %4d, displayer number = %4d\n",frame->ReadBytes,frame->TimeStamp);
            }
            break;
        case 1:
            if (audiofp != NULL)
            {
                fwrite(frame->Buffer,1,frame->FrameLen,audiofp);
                //printf("read size = %d\n",frame->ReadBytes);
            }
            break;
        default: ;
            //printf("ERROR\n");
    }
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    
    
    
    NSString *home = NSHomeDirectory();
    NSString *pcmfile = [[NSString alloc] initWithString:[NSString stringWithFormat:@"tmp/audio.pcm"]];
    NSString *yuvfile = [[NSString alloc] initWithString:[NSString stringWithFormat:@"tmp/video.yuv"]];
    
    audio_dst_filename = [[home stringByAppendingPathComponent:pcmfile] UTF8String];
    video_dst_filename = [[home stringByAppendingPathComponent:yuvfile] UTF8String];
    
    
    
    unsigned long	GVE_FFDemux_Handle = 0;
    
    GVE_FFDemux_OperatePar  OperatePar = {0};
    
    GVE_FFDemux_OutPutInfo  OutPutInfo = {0};
    GVE_FFDemux_ConfigPar   ConfigPar  = {0};
    
    //≈‰÷√≤Œ ˝£¨∞¸¿®Œƒº˛√˚∫Õªÿµ˜∫Ø ˝◊¢≤·£ª
    //°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™
    
    NSString *path= [[NSBundle mainBundle] resourcePath];
    
    
    NSString *file = [[NSString alloc] initWithString:[NSString stringWithFormat:@"fileshare.flv"]];
    path = [path stringByAppendingPathComponent:file];

    
    ConfigPar.input_filename = [path UTF8String];
    ConfigPar.pfun = WriteDecodeDate;
    //°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™
    
    audiofp = fopen(audio_dst_filename, "wb");
    if (!audiofp) {
        NSLog(@"Could not open %s.", audio_dst_filename);
    }
    
    videofp = fopen(video_dst_filename, "wb");
    if (!videofp) {
        NSLog(@"Could not open %s.", video_dst_filename);
    }
    
    //≥ı ºªØ–≈œ¢£¨÷Æ∫Ûµ√µΩ∂‡√ΩÃÂŒƒº˛µƒ–≈œ¢£ª
    //°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™
    GVE_FFMPEG_Demux_Create(&GVE_FFDemux_Handle,&OperatePar,&ConfigPar,&OutPutInfo);
    //°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™
    
    // ‰≥ˆŒƒº˛º∞Ω‚¬Î ”∆µ“Ù∆µ–≈œ¢£ª
    //°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™
    NSLog(@"  Duration: ");
    NSLog(@"%02d:%02d:%02d.%02d", OutPutInfo.fileinfo.hours, OutPutInfo.fileinfo.mins, OutPutInfo.fileinfo.secs,
          OutPutInfo.fileinfo.us);
    NSLog(@ ", bitrate: ");
    NSLog(@ "%d kb/s\n", OutPutInfo.fileinfo.BitRate);  //¬Î¬
    //°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™
    
    //≥ı ºªØ–Ë“™µƒ ”∆µº∞“Ù∆µbuffer
    //°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™
    OperatePar.audio_dst_data = malloc(INBUF_SIZE *sizeof(char));
    if(OperatePar.audio_dst_data == NULL)
    {
        exit(1);
    }
    OperatePar.video_dst_len = OutPutInfo.videoinfo.Width * OutPutInfo.videoinfo.Height * 3 /2;
    OperatePar.video_dst_data = malloc(OperatePar.video_dst_len  *sizeof(char));
    if(OperatePar.video_dst_data == NULL)
    {
        exit(1);
    }
    //°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™
    
    //…Ë÷√seekŒª÷√
    //-------------------------------------------------------------------------------------------------------------
    //ConfigPar.seek_perscent = 0.52;
    
    //ø™ ºΩ‚¬Î∂‡√ΩÃÂŒƒº˛
    //---------------------------------------------------------------------------------------------------------------
    GVE_FFMPEG_Demux_Decoder(GVE_FFDemux_Handle,&OperatePar,&ConfigPar,&OutPutInfo);
    
    // 	Sleep(1000);
    //
    // 	GVE_FFMPEG_Demux_Suspend(GVE_FFDemux_Handle,&OperatePar,&ConfigPar,&OutPutInfo);
    //
    // 	Sleep(1000);
    // 	GVE_FFMPEG_Demux_Resume(GVE_FFDemux_Handle,&OperatePar,&ConfigPar,&OutPutInfo);
    
    sleep(40000);
    
    GVE_FFMPEG_Demux_Destroy(GVE_FFDemux_Handle);
    
    if (OperatePar.audio_dst_data != NULL)
    {
        free(OperatePar.audio_dst_data);
    }
    if (OperatePar.video_dst_data != NULL)
    {
        free(OperatePar.video_dst_data);
    }
    
    fclose(audiofp);
    fclose(videofp);
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end


