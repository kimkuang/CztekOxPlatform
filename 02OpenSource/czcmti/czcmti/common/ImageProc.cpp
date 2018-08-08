#include "ImageProc.h"
#include <memory.h>
ImageProc::ImageProc()
{

}

void ImageProc::verticalFlip(uchar *in, int w, int h)
{
    if (nullptr == in)
    {
        return;
    }

    uchar *tmp = new uchar[w];
    for (int j = 0; j < h; j++)
    {
        memcpy(tmp, in + j * w, w);
        memcpy(in + j * w, in + (h - j - 1) * w, w);
        memcpy(in + (h -j -1) * w, tmp, w);
    }

    delete[] tmp;
}

void ImageProc::rawToRaw8(const ushort *src, uchar *dst, int size, uchar bit)
{
    if ((nullptr == src) || (nullptr == dst))
    {
        return;
    }

    for (int i=0; i<size; ++i)
    {
        dst[i] = (src[i] >> bit) & 0xFF;
    }
}

void ImageProc::rawToRaw8(const ushort *src, uchar *dst, int w, int h, uchar bit)
{
    if ((nullptr == src) || (nullptr == dst))
    {
        return;
    }

    for (int i=0; i<w*h; i++)
    {
        dst[i] = (src[i]>>bit) & 0xff;
    }
}

void ImageProc::raw8ToRaw(const uchar *src, ushort *dst, int w, int h, uchar bit)
{
    if ((nullptr == src) || (nullptr == dst))
    {
        return;
    }

    for (int i=0; i<w*h; i++)
    {
        dst[i] = (src[i] << bit);
    }
}

void ImageProc::scaleOutRGBImage(const uchar *src, int sw, int sh, uchar *dst, int dw, int dh)
{
    if ((nullptr == src) || (nullptr == dst))
    {
        return;
    }

    /*******************双线性插值******************/
    for (int r = 0; r < dh; ++r)
    {
        for (int c = 0; c < dw; ++c)
        {
            double eor = (r*sh*1.0) / dh;
            double eoc = (c*sw*1.0) / dw;
            int ior = (int)eor;
            int ioc = (int)eoc;
            double ax = eoc - ioc;                      //在原图像中与a点的水平距离
            double ay = eor - ior;                      //在原图像中与a点的垂直距离
            int pta = ior*sw*3 + ioc * 3;               //数组位置偏移量，对应于图像的各像素点RGB的起点,相当于点A
            int ptb = ior*sw*3 + (ioc + 1) * 3;         //数组位置偏移量，对应于图像的各像素点RGB的起点,相当于点B
            int ptc = (ior + 1)*sw*3 + ioc * 3;         //数组位置偏移量，对应于图像的各像素点RGB的起点,相当于点C
            int ptd = (ior + 1)*sw*3 + (ioc + 1) * 3;   //数组位置偏移量，对应于图像的各像素点RGB的起点,相当于点D
            if ((ior + 1) >= (dh - 1))
            {
                ptc = pta;
                ptd = ptb;
            }

            if ((ioc +1) >= (dw - 1))
            {
                ptb = pta;
                ptd = ptc;
            }

            //f(x,y) = f(0,0)(1-x)(1-y)+f(1,0)x(1-y)+f(0,1)(1-x)y+f(1,1)xy
            int pt = r*dw*3 + c*3;                      //映射尺度变换图像数组位置偏移量
            double expa = (1 - ax) * (1 - ay);
            double expb = (1 - ay) * ax;
            double expc = (1 - ax) * ay;
            double expd = ax * ay;

            dst[pt] = src[pta] * expa + src[ptb] * expb + src[ptc] * expc + src[ptd] * expd;
            dst[pt+1] = src[pta+1] * expa + src[ptb+1] * expb + src[ptc+1] * expc + src[ptd+1] * expd;
            dst[pt+2] = src[pta+2] * expa + src[ptb+2] * expb + src[ptc+2] * expc + src[ptd+2] * expd;
        }
    }
}

void ImageProc::cutOutRGBImage(const uchar *src, int sw, int sh, uchar *dst, int dw, int dh)
{
    if ((nullptr == src) || (nullptr == dst))
    {
        return;
    }

    int x = (sw - dw) / 4 * 2;
    int y = (sh - dh) / 4 * 2;
    for (int r=0; r<dh; ++r)
    {
        for (int c=0; c<dw; ++c)
        {
            dst[(r*dw + c)*3] = src[((r+y)*sw + c + x)*3];
            dst[(r*dw + c)*3 + 1] = src[((r+y)*sw + c + x)*3 + 1];
            dst[(r*dw + c)*3 + 2] = src[((r+y)*sw + c + x)*3 + 2];
        }
    }
}

void ImageProc::rawToRgb24Demosaic(const uchar * src, uchar * dst, int w, int h, uchar fmt)
{
    switch(fmt)
    {
    case IMAGE_MODE_YCbYCr_RG_GB: //RGGB
        rawToRgb24DemosaicForRGGB(src, dst, w, h);  //待修改
        break;
    case IMAGE_MODE_YCrYCb_GR_BG: //GRBG
        rawToRgb24DemosaicForGRBG(src, dst, w, h);  //待修改
        break;
    case IMAGE_MODE_CbYCrY_GB_RG:   //GBRG
        rawToRgb24DemosaicForGBRG(src, dst, w, h);  //待修改
        break;
    case IMAGE_MODE_CrYCbY_BG_GR:  //BGGR
        rawToRgb24DemosaicForBGGR(src, dst, w, h);  //已经修改，demosaic之后图片不会flip
        break;
    default:
        break;
    }
}

void ImageProc::rawToRgb24DemosaicForBGGR(const uchar * src, uchar * dst, int w, int h)
{
    int x = 0,y = 0,i=0;
    int td = 0;

    //Corner case
    x = 0;
    y = 0;
    dst[y * w * 3 + x * 3 + 0] = src[0]; //B
    dst[y * w * 3 + x * 3 + 1] = ( src[1] + src[w] )/2;  //G
    dst[y * w * 3 + x * 3 + 2] =   src[w+1]; //R

    x = w -1;
    y = 0;
    dst[y * w * 3 + x * 3 + 0] = src[w-2];
    dst[y * w * 3 + x * 3 + 1] = src[w-1];
    dst[y * w * 3 + x * 3 + 2] = src[2*w-1];

    x = 0;
    y = h -1;
    dst[y * w * 3 + x * 3 + 0] = src[(h-2)*w];
    dst[y * w * 3 + x * 3 + 1] = src[w*(h-1)];
    dst[y * w * 3 + x * 3 + 2] = src[(h-1)*w+1];

    x = w -1;
    y = h -1;
    dst[y * w * 3 + x * 3 + 0] =  src[(h-1)*w-2];
    dst[y * w * 3 + x * 3 + 1] =  (src[h*w-2]+src[(h-1)*w-1])/2;
    dst[y * w * 3 + x * 3 + 2] =  src[h*w-1];


    //for boundary case
    for( x = 1; x < w -1; x++ )
    {
        if( x%2 == 1 )
        {
            y = 0;
            i = x;
            dst[ y * w * 3 + x * 3 + 0 ] = ( src[i-1] + src[i+1] )/2;
            dst[ y * w * 3 + x * 3 + 1 ] = src[i];
            dst[ y * w * 3 + x * 3 + 2 ] =  src[w+i];

            y = h - 1;
            i = (h-1)*w + x;
            dst[ y * w * 3 + x * 3 + 0 ] =  ( src[i-w-1] + src[i-w+1] )/2;
            dst[ y * w * 3 + x * 3 + 1 ] =  ( src[i-1] + src[i+1])/2;
            dst[ y * w * 3 + x * 3 + 2 ] =  src[i];
        }
        else
        {
            y = 0;
            i = x;
            dst[ y * w * 3 + x * 3 + 0 ] =  src[i];
            dst[ y * w * 3 + x * 3 + 1 ] =  (src[i-1] + src[i+1])/2 ;
            dst[ y * w * 3 + x * 3 + 2 ] =  ( src[i+w-1] + src[i+w+1] )/2;

            y = h - 1;
            i = (h-1)*w + x;
            dst[ y * w * 3 + x * 3 + 0 ] =  src[i-w];
            dst[ y * w * 3 + x * 3 + 1 ] =  src[i];
            dst[ y * w * 3 + x * 3 + 2 ] =  ( src[i-1] + src[i+1] )/2;
        }
    }

    for( y = 1; y < h - 1; y++ )
    {
        if( y%2 == 1 )
        {
            x = 0;
            i = y*w;

            dst[ y * w * 3 + x * 3 + 0 ] = (src[i-w]+src[i+w])/2;
            dst[ y * w * 3 + x * 3 + 1 ] =  src[i];
            dst[ y * w * 3 + x * 3 + 2 ] =  src[i+1];

            x = w -1;
            i = y*w + x;
            dst[ y * w * 3 + x * 3 + 0 ] =  (src[i-w-1]+src[i+w-1])/2;
            dst[ y * w * 3 + x * 3 + 1 ] =  ( src[i-w] + src[i+w])/2;
            dst[ y * w * 3 + x * 3 + 2 ] =  src[i];
        }
        else
        {
            x = 0;
            i = y*w;
            dst[ y * w * 3 + x * 3 + 0 ] =  src[i];
            dst[ y * w * 3 + x * 3 + 1 ] =  (src[i-w] + src[i+w])/2 ;
            dst[ y * w * 3 + x * 3 + 2 ] =  (src[i-w+1]+src[i+w+1])/2;

            x = w -1;
            i = y* w + x;
            dst[ y * w * 3 + x * 3 + 0 ] =  src[i-1];
            dst[ y * w * 3 + x * 3 + 1 ] = src[i];
            dst[ y * w * 3 + x * 3 + 2 ] =  (src[i-w]+src[i+w])/2;
        }
    }

    //sub boundary case
    for( y = 1, x = 1; x < w - 1; x++ )
    {
        i = y*w + x;
        if( x%2 == 1 )
        {
            dst[y * w * 3 + x * 3 + 0] = (src[i-w-1]+src[i-w+1]+ src[i+w-1] + src[i+w+1] )/4;
            dst[y * w * 3 + x * 3 + 1] = (src[i-1]+src[i+1]+src[i-w]+src[i+w])/4;
            dst[y * w * 3 + x * 3 + 2] = src[i];
        }
        else
        {
            dst[y * w * 3 + x * 3 + 0] = (src[i-w]+src[i+w])/2;
            dst[y * w * 3 + x * 3 + 1] = src[i];
            dst[y * w * 3 + x * 3 + 2] = ( src[i-1] + src[i+1])/2;
        }
    }

    for( y = h-2, x = 1; x < w - 1; x++ )
    {
        i = y*w + x;

        if( x%2 == 1 )
        {
            dst[ y * w * 3 + x * 3 + 0 ] =   ( src[i-1] + src[i+1])/2;
            dst[ y * w * 3 + x * 3 + 1 ] =   src[i];
            dst[ y * w * 3 + x * 3 + 2 ] =   (src[i-w]+src[i+w])/2;
        }
        else
        {
            dst[ y * w * 3 + x * 3 + 0 ] = src[i];
            dst[ y * w * 3 + x * 3 + 1 ] = (src[i-1]+src[i+1]+src[i-w]+src[i+w])/4;
            dst[ y * w * 3 + x * 3 + 2 ] = (src[i-w-1] + src[i-w+1]+src[i+w-1]+src[i+w+1])/4;
        }
    }

    for( x = 1, y = 2; y < h - 2; y++ )
    {
        i = y*w + x;

        if( y%2 == 1 )
        {
            dst[y * w * 3 + x * 3 + 0] = ( src[i-w-1] + src[i-w+1]+src[i+w-1]+src[i+w+1] )/4;
            dst[y * w * 3 + x * 3 + 1] = (src[i-1]+src[i+1]+src[i-w]+src[i+w])/4;
            dst[y * w * 3 + x * 3 + 2] = src[i];
        }
        else
        {
            dst[ y * w * 3 + x * 3 + 0 ] = (src[i-1] + src[i+1])/2;
            dst[ y * w * 3 + x * 3 + 1 ] =  src[i];
            dst[ y * w * 3 + x * 3 + 2 ] =  (src[i-w]+src[i+w])/2;
        }
    }

    for( x = w-2, y = 2; y < h - 2; y++ )
    {
        i = y*w + x;

        if( y%2 == 1 )
        {
            dst[y * w * 3 + x * 3 + 0] = (src[i-w]+src[i+w])/2;
            dst[y * w * 3 + x * 3 + 1] = src[i];
            dst[y * w * 3 + x * 3 + 2] = (src[i-1] + src[i+1])/2;
        }
        else
        {
            dst[ y * w * 3 + x * 3 + 0 ] = src[i];
            dst[ y * w * 3 + x * 3 + 1 ] = (src[i-1]+src[i+1]+src[i-w]+src[i+w])/4;
            dst[ y * w * 3 + x * 3 + 2 ] = (src[i-w-1] + src[i-w+1]+src[i+w-1]+src[i+w+1])/4;
        }
    }

    //other area
    for( y = 2; y < h-2; y++ )
    {
        for( x = 2; x < w-2; x++ )
        {
            i = y*w + x;

            if( y%2 == 1 )
            {
                if( x%2 == 1 )
                {
                    td =  int(( src[i-w-1] + src[i-w+1]+src[i+w-1]+src[i+w+1] )*0.25 + (src[i]*0.75 -
                                                                                              (src[i-2*w]+src[i+2*w]+src[i-2]+src[i+2])*0.1875));

                    if( td < 0 )
                    {
                        td = 0;
                    }
                    else if( td > 255 )
                    {
                        td = 255;
                    }

                    dst[y * w * 3 + x * 3 + 0] =  td;

                    td = (int)(src[i-1]+src[i+1]+src[i-w]+src[i+w])*0.25 + (src[i]*0.5
                                                                                  -(src[i-2*w]+src[i+2*w]+src[i-2]+src[i+2])*0.125);

                    if( td < 0 )
                    {
                        td = 0;
                    }
                    else if( td > 255 )
                    {
                        td = 255;
                    }

                    dst[y * w * 3 + x * 3 + 1] = td;

                    dst[y * w * 3 + x * 3 + 2] = src[i];
                }
                else
                {
                    td = int((src[i-w] + src[i+w])*0.5 +
                            (src[i]*0.625+(src[i-2]+src[i+2])*0.0625 - (src[i-2*w]+src[i+2*w]+src[i-w-1]+src[i-w+1]+src[i+w-1]+src[i+w+1])*0.125));

                    if( td < 0 )
                    {
                        td = 0;
                    }
                    else if( td > 255 )
                    {
                        td = 255;
                    }

                    dst[y * w * 3 + x * 3 + 0] = td;
                    dst[y * w * 3 + x * 3 + 1] = src[i];

                    //
                    td = int(( src[i-1] + src[i+1])*0.5+
                            src[i]*0.625+(src[i-2*w]+src[i+2*w])*0.0625-(src[i-2]+src[i+2]+src[i-w-1]+src[i-w+1]+src[i+w-1]+src[i+w+1])*0.125);

                    if( td < 0 )
                    {
                        td = 0;
                    }
                    else if( td > 255 )
                    {
                        td = 255;
                    }

                    dst[y * w * 3 + x * 3 + 2] = td;

                }
            }
            else
            {
                if( x%2 == 1 )
                {
                    td =   ( src[i-1] + src[i+1])*0.5+
                            (src[i]*0.625+(src[i-2*w]+src[i+2*w])*0.0625-(src[i-2]+src[i+2]+src[i-w-1]+src[i-w+1]+src[i+w-1]+src[i+w+1])*0.125 );
                    if( td < 0 )
                    {
                        td = 0;
                    }
                    else if( td > 255 )
                    {
                        td = 255;
                    }

                    dst[ y * w * 3 + x * 3 + 0 ] = td;
                    dst[ y * w * 3 + x * 3 + 1 ] = src[i];

                    td =  int( ( src[i-w] + src[i+w] )*0.5+
                            (src[i]*0.625+(src[i-2]+src[i+2])*0.0625-(src[i-2*w]+src[i+2*w] + src[i-w-1]+src[i-w+1]+src[i+w-1]+src[i+w+1])*0.125));

                    if( td < 0 )
                    {
                        td = 0;
                    }
                    else if( td > 255 )
                    {
                        td = 255;
                    }

                    dst[ y * w * 3 + x * 3 + 2 ] = td;
                }
                else
                {
                    dst[ y * w * 3 + x * 3 + 0 ] =  src[i];

                    td  = int(  ( src[i-1] + src[i+1]+src[i-w]+src[i+w] )*0.25 +(src[i]*0.5-
                                                                                       (src[i-2]+src[i+2]+src[i-2*w]+src[i+2*w])*0.125));

                    if( td < 0 )
                    {
                        td = 0;
                    }
                    else if( td > 255 )
                    {
                        td = 255;
                    }

                    dst[ y * w * 3 + x * 3 + 1 ] = td;

                    td =  int( ( src[i-w-1] + src[i-w+1]+src[i+w-1]+src[i+w+1] )*0.25+
                            (src[i]*0.75 -(src[i-2*w]+src[i+2*w]+src[i-2]+src[i+2])*0.1875));

                    if( td < 0 )
                    {
                        td = 0;
                    }
                    else if( td > 255 )
                    {
                        td = 255;
                    }

                    dst[ y * w * 3 + x * 3 + 2 ] = td;

                }
            }
        }
    }
}

void ImageProc::rawToRgb24DemosaicForGBRG(const uchar * src, uchar * dst, int w, int h)
{
    int x = 0,y = 0,i=0;
    int td = 0;
    //////////////////////////////////////
    {
        //Corner case
        x = 0;
        y = 0;
        dst[y * w * 3 + x * 3 + 0] = src[1]; //B
        dst[y * w * 3 + x * 3 + 1] = src[0];
        dst[y * w * 3 + x * 3 + 2] = src[w]; //R

        x = w -1;
        y = 0;
        dst[y * w * 3 + x * 3 + 0] = src[w-1];
        dst[y * w * 3 + x * 3 + 1] = (src[w-2]+src[2*w-1])/2;
        dst[y * w * 3 + x * 3 + 2] = src[2*w-2];

        x = 0;
        y = h -1;
        dst[y * w * 3 + x * 3 + 0] = src[(h-2)*w+1];
        dst[y * w * 3 + x * 3 + 1] = (src[(h-2)*w]+src[(h-1)*w+1])/2;
        dst[y * w * 3 + x * 3 + 2] = src[(h-1)*w];

        x = w -1;
        y = h -1;
        dst[y * w * 3 + x * 3 + 0] =  src[(h-1)*w-1];
        dst[y * w * 3 + x * 3 + 1] = src[h*w-1];
        dst[y * w * 3 + x * 3 + 2] =  src[h*w-2];


        //for boundary case
        for( x = 1; x < w -1; x++ )
        {
            if( x%2 == 1 )
            {
                y = 0;
                i = x;
                dst[ y * w * 3 + x * 3 + 0 ] = src[i];
                dst[ y * w * 3 + x * 3 + 1 ] = ( src[i-1] + src[i+1] )/2;
                dst[ y * w * 3 + x * 3 + 2 ] = (src[i+w-1]+src[i+w+1])/2;


                y = h - 1;
                i = (h-1)*w + x;
                dst[ y * w * 3 + x * 3 + 0 ] = src[i-w];
                dst[ y * w * 3 + x * 3 + 1 ] = src[i];
                dst[ y * w * 3 + x * 3 + 2 ] = ( src[i-1] + src[i+1] )/2;

            }
            else
            {
                y = 0;
                i = x;
                dst[ y * w * 3 + x * 3 + 0 ] = (src[i-1]+src[i+1])/2;
                dst[ y * w * 3 + x * 3 + 1 ] = src[i];
                dst[ y * w * 3 + x * 3 + 2 ] = src[i+w];


                y = h - 1;
                i = (h-1)*w + x;
                dst[ y * w * 3 + x * 3 + 0 ] = (src[i-w-1]+src[i-w+1])/2;
                dst[ y * w * 3 + x * 3 + 1 ] = (src[i-1]+src[i+1])/2;
                dst[ y * w * 3 + x * 3 + 2 ] = src[i];
            }
        }

        for( y = 1; y < h - 1; y++ )
        {
            if( y%2 == 1 )
            {
                x = 0;
                i = y*w;

                dst[ y * w * 3 + x * 3 + 0 ] = (src[i-w+1]+src[i+w+1])/2;
                dst[ y * w * 3 + x * 3 + 1 ] = (src[i-w]+src[i+w])/2;
                dst[ y * w * 3 + x * 3 + 2 ] =  src[i];


                x = w -1;
                i = y*w + x;
                dst[ y * w * 3 + x * 3 + 0 ] =  (src[i-w]+src[i+w])/2;
                dst[ y * w * 3 + x * 3 + 1 ] =  src[i];
                dst[ y * w * 3 + x * 3 + 2 ] =  src[i-1];

            }
            else
            {
                x = 0;
                i = y*w;
                dst[ y * w * 3 + x * 3 + 0 ] =  src[i+1];
                dst[ y * w * 3 + x * 3 + 1 ] =  src[i] ;
                dst[ y * w * 3 + x * 3 + 2 ] =  (src[i-w]+src[i+w])/2;


                x = w -1;
                i = y* w + x;
                dst[ y * w * 3 + x * 3 + 0 ] =  src[i];
                dst[ y * w * 3 + x * 3 + 1 ] = (src[i-w]+src[i+w])/2;
                dst[ y * w * 3 + x * 3 + 2 ] =  (src[i-w-1]+src[i-w+1])/2;
            }
        }

        //sub boundary case
        for( y = 1, x = 1; x < w - 1; x++ )
        {
            i = y*w + x;
            if( x%2 == 1 )
            {
                dst[y * w * 3 + x * 3 + 0] = (src[i-w]+ src[i+w] )/2;
                dst[y * w * 3 + x * 3 + 1] = src[i];
                dst[y * w * 3 + x * 3 + 2] = (src[i-1]+src[i+1])/2;

            }
            else
            {
                dst[y * w * 3 + x * 3 + 0] = (src[i-w-1]+src[i-w+1]+src[i+w-1]+src[i+w+1])/4;
                dst[y * w * 3 + x * 3 + 1] = (src[i-w]+src[i+w]+src[i-1]+src[i+1])/4;
                dst[y * w * 3 + x * 3 + 2] = src[i];
            }
        }

        for( y = h-2, x = 1; x < w - 1; x++ )
        {
            i = y*w + x;

            if( x%2 == 1 )
            {
                dst[ y * w * 3 + x * 3 + 0 ] =   src[i];
                dst[ y * w * 3 + x * 3 + 1 ] =   (src[i-w]+src[i+w]+src[i-1]+src[i+1])/4;
                dst[ y * w * 3 + x * 3 + 2 ] =   (src[i-w-1]+src[i-w+1]+src[i+w-1]+src[i+w+1])/4;

            }
            else
            {
                dst[ y * w * 3 + x * 3 + 0 ] = (src[i-1]+src[i+1])/2;
                dst[ y * w * 3 + x * 3 + 1 ] = src[i];
                dst[ y * w * 3 + x * 3 + 2 ] = (src[i-w]+src[i+w])/2;
            }
        }

        for( x = 1, y = 2; y < h - 2; y++ )
        {
            i = y*w + x;

            if( y%2 == 1 )
            {
                dst[y * w * 3 + x * 3 + 0] = ( src[i-w]+src[i+w] )/2;
                dst[y * w * 3 + x * 3 + 1] = src[i];
                dst[y * w * 3 + x * 3 + 2] = (src[i-1]+src[i+1])/2;
            }
            else
            {
                dst[ y * w * 3 + x * 3 + 0 ] = src[i];
                dst[ y * w * 3 + x * 3 + 1 ] =  (src[i-w]+src[i+w]+src[i-1]+src[i+1])/4;
                dst[ y * w * 3 + x * 3 + 2 ] =  (src[i-w-1]+src[i-w+1]+src[i+w-1]+src[i+w+1])/4;
            }
        }

        for( x = w-2, y = 2; y < h - 2; y++ )
        {
            i = y*w + x;

            if( y%2 == 1 )
            {
                dst[y * w * 3 + x * 3 + 0] = (src[i-w-1]+src[i-w+1]+src[i+w-1]+src[i+w+1])/4;
                dst[y * w * 3 + x * 3 + 1] =  (src[i-w]+src[i+w]+src[i-1]+src[i+1])/4;
                dst[y * w * 3 + x * 3 + 2] = src[i];
            }
            else
            {
                dst[ y * w * 3 + x * 3 + 0 ] = (src[i-1]+src[i+1])/2;
                dst[ y * w * 3 + x * 3 + 1 ] = src[i];
                dst[ y * w * 3 + x * 3 + 2 ] = (src[i-w]+src[i+w])/2;
            }
        }

        //other area
        for( y = 2; y < h-2; y++ )
        {
            for( x = 2; x < w-2; x++ )
            {
                i = y*w + x;

                if( y%2 == 1 )
                {
                    if( x%2 == 1 )
                    {
// 						td =  ( src[i-w] + src[i+w])*0.5+(src[i]*0.625+(src[i-2]+src[i+2])*0.0625-
// 							(src[i-2*w]+src[i+2*w]+src[i-w-1]+src[i-w+1]+src[i+w-1]+src[i+w+1])*0.125);
                        //B
                        td = (src[i - w] + src[i + w]) * 0.5 +
                            src[i] * 0.625 +
                            (src[i - 2] + src[i + 2]) * 0.0625 -
                            (src[i - 2 * w] + src[i + 2 * w] + src[i - w - 1] + src[i - w + 1] + src[i + w - 1] + src[i + w + 1]) * 0.125;

                        if( td < 0 )
                        {
                            td = 0;
                        }
                        else if( td > 255 )
                        {
                            td = 255;
                        }
                        dst[y * w * 3 + x * 3 + 0] =  td;

                        //G
                        dst[y * w * 3 + x * 3 + 1] = src[i];

                        //R
                        td = (src[i - 1] + src[i + 1]) * 0.5 +
                            src[i] * 0.625 +
                            (src[i - 2 * w] + src[i + 2 * w]) * 0.0625 -
                            (src[i - 2] + src[i + 2] + src[i - w - 1] + src[i + w + 1] + src[i + w - 1]+src[i + w + 1])*0.125;

                        if( td < 0 )
                        {
                            td = 0;
                        }
                        else if( td > 255 )
                        {
                            td = 255;
                        }
                        dst[y * w * 3 + x * 3 + 2] =  td;
                    }
                    else
                    {
                        //B
                        td = (src[i - w - 1] + src[i - w + 1] + src[i + w -1] + src[i + w + 1]) * 0.25 +
                            src[i] * 0.75 -
                            (src[i - 2 * w] + src[i + 2 * w] + src[i - 2] + src[i + 2]) * 0.1875;
                        if( td < 0 )
                        {
                            td = 0;
                        }
                        else if( td > 255 )
                        {
                            td = 255;
                        }
                        dst[y * w * 3 + x * 3 + 0] = td;

                        //G
                        td = (src[i - 1] + src[i + 1] + src[i - w] + src[i + w]) * 0.25 +
                            src[i] * 0.5 -
                            (src[i - 2 * w] + src[i + 2 * w] + src[i - 2] + src[i + 2]) * 0.125;
                        if( td < 0 )
                        {
                            td = 0;
                        }
                        else if( td > 255 )
                        {
                            td = 255;
                        }
                        dst[y * w * 3 + x * 3 + 1] = td;

                        //R
                        dst[y * w * 3 + x * 3 + 2] = src[i];
                    }
                }
                else
                {
                    if( x%2 == 1 )
                    {
                        //B
                        dst[ y * w * 3 + x * 3 + 0] = src[i];

                        //G
                        td = (src[i - 1] + src[i + 1] + src[i - w] + src[i + w]) * 0.25 +
                            src[i] * 0.5 -
                            (src[i - 2 * w] + src[i + 2 * w] + src[i - 2] + src[i + 2]) * 0.125;
                        if( td < 0 )
                        {
                            td = 0;
                        }
                        else if( td > 255 )
                        {
                            td = 255;
                        }
                        dst[ y * w * 3 + x * 3 + 1 ] = td;

                        //R
                        td = (src[i - w - 1] + src[i - w + 1] + src[i + w - 1] + src[i + w + 1]) * 0.25 +
                            src[i] * 0.75 -
                            (src[i - 2 * w] + src[i + 2 * w] + src[i - 2] + src[i + 2]) * 0.1875;

                        if( td < 0 )
                        {
                            td = 0;
                        }
                        else if( td > 255 )
                        {
                            td = 255;
                        }
                        dst[ y * w * 3 + x * 3 + 2] = td;
                    }
                    else
                    {
                        //B
                        td  = (src[i - 1] + src[i + 1]) * 0.5 +
                            src[i] * 0.625 +
                            (src[i - 2 * w] + src[i + 2 * w]) * 0.0625 -
                            (src[i - 2] + src[i + 2] + src[i - w - 1] + src[i - w + 1] + src[i + w - 1] + src[i + w + 1]) * 0.125;

                        if( td < 0 )
                        {
                            td = 0;
                        }
                        else if( td > 255 )
                        {
                            td = 255;
                        }
                        dst[ y * w * 3 + x * 3 + 0 ] = td;

                        //G
                        dst[ y * w * 3 + x * 3 + 1 ] = src[i];

                        //R
                        td = (src[i - w] + src[i + w]) * 0.5 +
                            src[i] * 0.625 +
                            (src[i - 2] + src[i + 2]) * 0.0625 -
                            (src[i - 2] + src[i + 2] + src[i - w - 1] + src[i - w + 1] + src[i + w - 1] + src[i + w + 1]) * 0.125;
                        if( td < 0 )
                        {
                            td = 0;
                        }
                        else if( td > 255 )
                        {
                            td = 255;
                        }
                        dst[ y * w * 3 + x * 3 + 2 ] = td;
                    }
                }
            }
        }
    }
}

void ImageProc::rawToRgb24DemosaicForGRBG(const uchar * src, uchar * dst, int w, int h)
{
    int x = 0,y = 0,i=0;
    int td = 0;
    //////////////////////////////////////
    {
        //Corner case
        x = 0;
        y = 0;
        dst[y * w * 3 + x * 3 + 0] = src[w]; //B
        dst[y * w * 3 + x * 3 + 1] = src[0];  //G
        dst[y * w * 3 + x * 3 + 2] = src[1]; //R

        x = w -1;
        y = 0;
        dst[y * w * 3 + x * 3 + 0] = src[2*w-2];
        dst[y * w * 3 + x * 3 + 1] = (src[w-2]+src[2*w-1])/2;
        dst[y * w * 3 + x * 3 + 2] = src[w-1];

        x = 0;
        y = h -1;
        dst[y * w * 3 + x * 3 + 0] = src[(h-1)*w];
        dst[y * w * 3 + x * 3 + 1] = (src[(h-2)*w]+src[(h-1)*w+1])/2;
        dst[y * w * 3 + x * 3 + 2] = src[(h-2)*w+1];

        x = w -1;
        y = h -1;
        dst[y * w * 3 + x * 3 + 0] =  src[h*w-2];
        dst[y * w * 3 + x * 3 + 1] =  src[h*w-1];
        dst[y * w * 3 + x * 3 + 2] =  src[(h-1)*w-1];

        //for boundary case
        for( x = 1; x < w -1; x++ )
        {
            if( x%2 == 1 )
            {
                y = 0;
                i = x;
                dst[ y * w * 3 + x * 3 + 0 ] = ( src[i+w-1] + src[i+w+1] )/2;
                dst[ y * w * 3 + x * 3 + 1 ] = (src[i-1]+src[i+1])/2;
                dst[ y * w * 3 + x * 3 + 2 ] =  src[i];

                y = h - 1;
                i = (h-1)*w + x;
                dst[ y * w * 3 + x * 3 + 0 ] =  ( src[i-1] + src[i+1] )/2;
                dst[ y * w * 3 + x * 3 + 1 ] =  src[i];
                dst[ y * w * 3 + x * 3 + 2 ] =  src[i-w];
            }
            else
            {
                y = 0;
                i = x;
                dst[ y * w * 3 + x * 3 + 0 ] =  src[i+w];
                dst[ y * w * 3 + x * 3 + 1 ] =  src[i];
                dst[ y * w * 3 + x * 3 + 2 ] =  ( src[i-1] + src[i+1] )/2;

                y = h - 1;
                i = (h-1)*w + x;
                dst[ y * w * 3 + x * 3 + 0 ] =  src[i];
                dst[ y * w * 3 + x * 3 + 1 ] =  (src[i-1]+src[i+1])/2;
                dst[ y * w * 3 + x * 3 + 2 ] =  ( src[i-w-1] + src[i-w+1] )/2;
            }
        }

        for( y = 1; y < h - 1; y++ )
        {
            if( y%2 == 1 )
            {
                x = 0;
                i = y*w;

                dst[ y * w * 3 + x * 3 + 0 ] = src[i];
                dst[ y * w * 3 + x * 3 + 1 ] = (src[i-w]+src[i+w])/2;
                dst[ y * w * 3 + x * 3 + 2 ] = (src[i-w+1]+src[i+w+1])/2;

                x = w -1;
                i = y*w + x;
                dst[ y * w * 3 + x * 3 + 0 ] =  src[i-1];
                dst[ y * w * 3 + x * 3 + 1 ] =  src[i];
                dst[ y * w * 3 + x * 3 + 2 ] =  (src[i-w]+src[i+w])/2;
            }
            else
            {
                x = 0;
                i = y*w;
                dst[ y * w * 3 + x * 3 + 0 ] =  (src[i-w]+src[i+w])/2;
                dst[ y * w * 3 + x * 3 + 1 ] =  src[i] ;
                dst[ y * w * 3 + x * 3 + 2 ] =  src[i+1];

                x = w -1;
                i = y* w + x;
                dst[ y * w * 3 + x * 3 + 0 ] =  (src[i-w-1]+src[i+w-1])/2;
                dst[ y * w * 3 + x * 3 + 1 ] = (src[i-w]+src[i+w])/2;
                dst[ y * w * 3 + x * 3 + 2 ] =  src[i];
            }
        }

        //sub boundary case
        for( y = 1, x = 1; x < w - 1; x++ )
        {
            i = y*w + x;
            if( x%2 == 1 )
            {
                dst[y * w * 3 + x * 3 + 0] = (src[i-1]+src[i+1])/2;
                dst[y * w * 3 + x * 3 + 1] = src[i];
                dst[y * w * 3 + x * 3 + 2] = (src[i-w]+src[i+w])/2;
            }
            else
            {
                dst[y * w * 3 + x * 3 + 0] = src[i];
                dst[y * w * 3 + x * 3 + 1] = (src[i-w]+src[i+w]+src[i-1]+src[i+1])/4;
                dst[y * w * 3 + x * 3 + 2] = ( src[i-w-1] + src[i-w+1]+src[i+w-1]+src[i+w+1])/4;
            }
        }

        for( y = h-2, x = 1; x < w - 1; x++ )
        {
            i = y*w + x;

            if( x%2 == 1 )
            {
                dst[ y * w * 3 + x * 3 + 0 ] =   ( src[i-w-1] + src[i-w+1]+src[i+w-1]+src[i+w+1])/4;
                dst[ y * w * 3 + x * 3 + 1 ] =   (src[i-w]+src[i+w]+src[i-1]+src[i+1])/4;
                dst[ y * w * 3 + x * 3 + 2 ] =   src[i];
            }
            else
            {
                dst[ y * w * 3 + x * 3 + 0 ] = (src[i-w]+src[i+w])/2;
                dst[ y * w * 3 + x * 3 + 1 ] = src[i];
                dst[ y * w * 3 + x * 3 + 2 ] = (src[i-1] + src[i+1])/2;
            }
        }

        for( x = 1, y = 2; y < h - 2; y++ )
        {
            i = y*w + x;

            if( y%2 == 1 )
            {
                dst[y * w * 3 + x * 3 + 0] = ( src[i-1] + src[i+1])/2;
                dst[y * w * 3 + x * 3 + 1] = src[i];
                dst[y * w * 3 + x * 3 + 2] = (src[i-w]+src[i+w])/2;
            }
            else
            {
                dst[ y * w * 3 + x * 3 + 0 ] = (src[i-w-1] + src[i-w+1]+src[i+w-1]+src[i+w+1])/4;
                dst[ y * w * 3 + x * 3 + 1 ] =  (src[i-w]+src[i+w]+src[i-1]+src[i+1])/4;
                dst[ y * w * 3 + x * 3 + 2 ] =  src[i];
            }
        }

        for( x = w-2, y = 2; y < h - 2; y++ )
        {
            i = y*w + x;

            if( y%2 == 1 )
            {
                dst[y * w * 3 + x * 3 + 0] = src[i];
                dst[y * w * 3 + x * 3 + 1] = (src[i-w]+src[i+w]+src[i-1]+src[i+1])/4;
                dst[y * w * 3 + x * 3 + 2] = (src[i-w-1] + src[i-w+1]+src[i+w-1]+src[i+w+1])/4;
            }
            else
            {
                dst[ y * w * 3 + x * 3 + 0 ] = (src[i-w]+src[i+w])/2;
                dst[ y * w * 3 + x * 3 + 1 ] = src[i];
                dst[ y * w * 3 + x * 3 + 2 ] = (src[i-1] + src[i+1])/2;
            }
        }

        //other area
        for( y = 2; y < h-2; y++ )
        {
            for( x = 2; x < w-2; x++ )
            {
                i = y*w + x;

                if( y%2 == 1 )
                {
                    if( x%2 == 1 )
                    {
                        //B
                        td =  (src[i - 1] + src[i + 1]) * 0.5 +
                            src[i] * 0.625 +
                            (src[i - 2 * w] + src[i + 2 * w]) * 0.0625 -
                            (src[i - 2] + src[i + 2] + src[i - w - 1] + src[i - w + 1] + src[i + w - 1] + src[i + w + 1]) * 0.125;

                        if( td < 0 )
                        {
                            td = 0;
                        }
                        else if( td > 255 )
                        {
                            td = 255;
                        }
                        dst[y * w * 3 + x * 3 + 0] =  td;

                        //G
                        dst[y * w * 3 + x * 3 + 1] = src[i];

                        //R
                        td = (src[i - w] + src[i + w]) * 0.5 +
                            src[i] * 0.625 +
                            (src[i - 2] + src[i + 2]) * 0.0625 -
                            (src[i - 2 * w] + src[i + 2 * w] + src[i - w - 1] + src[i - w + 1] + src[i + w - 1] + src[i + w + 1]) * 0.125;

                        if( td < 0 )
                        {
                            td = 0;
                        }
                        else if( td > 255 )
                        {
                            td = 255;
                        }
                        dst[y * w * 3 + x * 3 + 2] = td;
                    }
                    else
                    {
                        //B
                        dst[y * w * 3 + x * 3 + 0] = src[i];

                        //G
                        td = (src[i - 1] + src[i + 1] + src[i - w] + src[i + w]) * 0.25 +
                            src[i] * 0.5 -
                            (src[i - 2 * w] + src[i + 2 * w] + src[i - 2] + src[i + 2]) * 0.125;
                        if( td < 0 )
                        {
                            td = 0;
                        }
                        else if( td > 255 )
                        {
                            td = 255;
                        }
                        dst[y * w * 3 + x * 3 + 1] = td;

                        //R
                        td = (src[i - w - 1] + src[i - w + 1] + src[i + w - 1] + src[i + w + 1]) * 0.25 +
                            src[i] * 0.75 -
                            (src[i - 2 * w] + src[i + 2 * w] + src[i - 2] + src[i + 2]) * 0.1875;

                        if( td < 0 )
                        {
                            td = 0;
                        }
                        else if( td > 255 )
                        {
                            td = 255;
                        }
                        dst[y * w * 3 + x * 3 + 2] = td;
                    }
                }
                else
                {
                    if( x%2 == 1 )
                    {
                        //B
                        td = (src[i - w - 1] + src[i - w + 1] + src[i + w - 1] + src[i + w + 1]) * 0.25 +
                            src[i] * 0.625 -
                            (src[i - 2 * w] + src[i + 2 * w] + src[i - 2] + src[i + 2]) * 0.1875;
                        if( td < 0 )
                        {
                            td = 0;
                        }
                        else if( td > 255 )
                        {
                            td = 255;
                        }
                        dst[ y * w * 3 + x * 3 + 0] = td;

                        //G
                        td = (src[i - 1] + src[i + 1] + src[i - w] + src[i + w]) * 0.25 +
                            src[i] * 0.5 -
                            (src[i - 2 * w] + src[i + 2 * w] + src[i - 2] + src[i + 2]) * 0.125;

                        if( td < 0 )
                        {
                            td = 0;
                        }
                        else if( td > 255 )
                        {
                            td = 255;
                        }
                        dst[ y * w * 3 + x * 3 + 1] = td;

                        //R
                        dst[ y * w * 3 + x * 3 + 2] = src[i];
                    }
                    else
                    {
                        //B
                        td  = (src[i - w - 1] + src[i - w + 1] + src[i + w - 1] + src[i + w + 1]) * 0.25 +
                            src[i] * 0.75 -
                            (src[i - 2 * w] + src[i + 2 * w] + src[i - 2] + src[i +2]) * 0.1875;

                        if( td < 0 )
                        {
                            td = 0;
                        }
                        else if( td > 255 )
                        {
                            td = 255;
                        }
                        dst[ y * w * 3 + x * 3 + 0 ] = td;

                        //G
                        td = (src[i - 1] + src[i + 1] + src[i - w] + src[i + w]) * 0.25 +
                            src[i] * 0.5 -
                            (src[i - 2 * w] + src[i + 2 * w] + src[i - 2] + src[i + 2]) * 0.125;

                        if( td < 0 )
                        {
                            td = 0;
                        }
                        else if( td > 255 )
                        {
                            td = 255;
                        }
                        dst[ y * w * 3 + x * 3 + 2 ] = td;

                        //R
                        dst[ y * w * 3 + x * 3 + 1 ] = src[i];
                    }
                }
            }
        }
    }
}

void ImageProc::rawToRgb24DemosaicForRGGB(const uchar * src, uchar * dst, int w, int h)
{
    int x = 0,y = 0,i=0;
    int td = 0;
    //////////////////////////////////////
    {
        //Corner case
        x = 0;
        y = 0;
        dst[y * w * 3 + x * 3 + 0] = src[w + 1]; //B
        dst[y * w * 3 + x * 3 + 1] = (src[1] + src[w]) / 2;  //G
        dst[y * w * 3 + x * 3 + 2] = src[0]; //R

        x = w -1;
        y = 0;
        dst[y * w * 3 + x * 3 + 0] = src[2 * w - 1];
        dst[y * w * 3 + x * 3 + 1] = src[w-1];
        dst[y * w * 3 + x * 3 + 2] = src[w-2];

        x = 0;
        y = h -1;
        dst[y * w * 3 + x * 3 + 0] = src[(h-1)*w+1];
        dst[y * w * 3 + x * 3 + 1] = src[(h-1)*w];
        dst[y * w * 3 + x * 3 + 2] = src[(h-2)*w];

        x = w -1;
        y = h -1;
        dst[y * w * 3 + x * 3 + 0] =  src[h*w-1];
        dst[y * w * 3 + x * 3 + 1] =  (src[h*w-2]+src[(h-1)*w-1])/2;
        dst[y * w * 3 + x * 3 + 2] =  src[(h-1)*w-2];


        //for boundary case
        for( x = 1; x < w -1; x++ )
        {
            if( x%2 == 1 )
            {
                y = 0;
                i = x;
                dst[ y * w * 3 + x * 3 + 0 ] = src[i+w];
                dst[ y * w * 3 + x * 3 + 1 ] = src[i];
                dst[ y * w * 3 + x * 3 + 2 ] = (src[i-1]+src[i+1])/2;

                y = h - 1;
                i = (h-1)*w + x;
                dst[ y * w * 3 + x * 3 + 0 ] = src[i];
                dst[ y * w * 3 + x * 3 + 1 ] =  ( src[i-1] + src[i+1])/2;
                dst[ y * w * 3 + x * 3 + 2 ] =  (src[i-w-1]+src[i-w+1])/2;
            }
            else
            {
                y = 0;
                i = x;
                dst[ y * w * 3 + x * 3 + 0 ] =  (src[i+w-1]+src[i+w+1])/2;
                dst[ y * w * 3 + x * 3 + 1 ] =  (src[i-1] + src[i+1])/2 ;
                dst[ y * w * 3 + x * 3 + 2 ] =  src[i];

                y = h - 1;
                i = (h-1)*w + x;
                dst[ y * w * 3 + x * 3 + 0 ] =  (src[i-1]+src[i+1])/2;
                dst[ y * w * 3 + x * 3 + 1 ] =  src[i];
                dst[ y * w * 3 + x * 3 + 2 ] =  src[i-w];
            }
        }

        for( y = 1; y < h - 1; y++ )
        {
            if( y%2 == 1 )
            {
                x = 0;
                i = y*w;

                dst[ y * w * 3 + x * 3 + 0 ] = src[i+1];
                dst[ y * w * 3 + x * 3 + 1 ] =  src[i];
                dst[ y * w * 3 + x * 3 + 2 ] =  (src[i-w]+src[i+w])/2;

                x = w -1;
                i = y*w + x;
                dst[ y * w * 3 + x * 3 + 0 ] =  src[i];
                dst[ y * w * 3 + x * 3 + 1 ] =  ( src[i-w] + src[i+w])/2;
                dst[ y * w * 3 + x * 3 + 2 ] =  (src[i-1-w]+src[i-1+w])/2;
            }
            else
            {
                x = 0;
                i = y*w;
                dst[ y * w * 3 + x * 3 + 0 ] =  (src[i+1-w]+src[i+1+w])/2;
                dst[ y * w * 3 + x * 3 + 1 ] =  (src[i-w] + src[i+w])/2 ;
                dst[ y * w * 3 + x * 3 + 2 ] =  src[i];

                x = w -1;
                i = y* w + x;
                dst[ y * w * 3 + x * 3 + 0 ] =  (src[i-w]+src[i+w])/2;
                dst[ y * w * 3 + x * 3 + 1 ] = src[i];
                dst[ y * w * 3 + x * 3 + 2 ] = src[i-1];
            }
        }

        //sub boundary case
        for( y = 1, x = 1; x < w - 1; x++ )
        {
            i = y*w + x;
            if( x%2 == 1 )
            {
                dst[y * w * 3 + x * 3 + 0] = src[i];
                dst[y * w * 3 + x * 3 + 1] = (src[i-1]+src[i+1]+src[i-w]+src[i+w])/4;
                dst[y * w * 3 + x * 3 + 2] = (src[i-w-1]+src[i-w+1]+src[i+w-1]+src[i+w+1])/4;
            }
            else
            {
                dst[y * w * 3 + x * 3 + 0] = (src[i-1]+src[i+1])/2;
                dst[y * w * 3 + x * 3 + 1] = src[i];
                dst[y * w * 3 + x * 3 + 2] = ( src[i-w] + src[i+w])/2;
            }
        }

        for( y = h-2, x = 1; x < w - 1; x++ )
        {
            i = y*w + x;

            if( x%2 == 1 )
            {
                dst[ y * w * 3 + x * 3 + 0 ] =   ( src[i-w] + src[i+w])/2;
                dst[ y * w * 3 + x * 3 + 1 ] =   src[i];
                dst[ y * w * 3 + x * 3 + 2 ] =   (src[i-1]+src[i+1])/2;
            }
            else
            {
                dst[ y * w * 3 + x * 3 + 0 ] = (src[i-w-1] + src[i-w+1]+src[i+w-1]+src[i+w+1])/4;
                dst[ y * w * 3 + x * 3 + 1 ] = (src[i-1]+src[i+1]+src[i-w]+src[i+w])/4;
                dst[ y * w * 3 + x * 3 + 2 ] = src[i];
            }
        }

        for( x = 1, y = 2; y < h - 2; y++ )
        {
            i = y*w + x;

            if( y%2 == 1 )
            {
                dst[y * w * 3 + x * 3 + 0] = src[i];
                dst[y * w * 3 + x * 3 + 1] = (src[i-1]+src[i+1]+src[i-w]+src[i+w])/4;
                dst[y * w * 3 + x * 3 + 2] = ( src[i-w-1] + src[i-w+1]+src[i+w-1]+src[i+w+1] )/4;
            }
            else
            {
                dst[ y * w * 3 + x * 3 + 0 ] = (src[i-w] + src[i+w])/2;
                dst[ y * w * 3 + x * 3 + 1 ] =  src[i];
                dst[ y * w * 3 + x * 3 + 2 ] =  (src[i-1]+src[i+1])/2;
            }
        }

        for( x = w-2, y = 2; y < h - 2; y++ )
        {
            i = y*w + x;

            if( y%2 == 1 )
            {
                dst[y * w * 3 + x * 3 + 0] = (src[i-1]+src[i+1])/2;
                dst[y * w * 3 + x * 3 + 1] = src[i];
                dst[y * w * 3 + x * 3 + 2] = (src[i-w] + src[i+w])/2;
            }
            else
            {
                dst[ y * w * 3 + x * 3 + 0 ] = (src[i-w-1] + src[i-w+1]+src[i+w-1]+src[i+w+1])/4;
                dst[ y * w * 3 + x * 3 + 1 ] = (src[i-1]+src[i+1]+src[i-w]+src[i+w])/4;
                dst[ y * w * 3 + x * 3 + 2 ] = src[i];
            }
        }

        //other area
        for( y = 2; y < h-2; y++ )
        {
            for( x = 2; x < w-2; x++ )
            {
                i = y*w + x;

                if( y%2 == 1 )
                {
                    if( x%2 == 1 )
                    {
                        dst[y * w * 3 + x * 3 + 0] =  src[i];

                        td = (src[i-1]+src[i+1]+src[i-w]+src[i+w])*0.25 + src[i]*0.5 -
                            (src[i-2*w]+src[i+2*w]+src[i-2]+src[i+2])*0.125;

                        if( td < 0 )
                        {
                            td = 0;
                        }
                        else if( td > 255 )
                        {
                            td = 255;
                        }

                        dst[y * w * 3 + x * 3 + 1] = td;

                        td = (src[i-w-1]+src[i-w+1]+src[i+w-1]+src[i+w+1])*0.25 + src[i]*0.75 -
                            (src[i-2*w]+src[i+2*w]+src[i-2]+src[i+2])*0.1875;

                        if( td < 0 )
                        {
                            td = 0;
                        }
                        else if( td > 255 )
                        {
                            td = 255;
                        }

                        dst[y * w * 3 + x * 3 + 2] = td;
                    }
                    else
                    {
                        td = (src[i-1]+src[i+1])*0.5 + src[i]*0.625 + (src[i-2*w]+src[i+2*w])*0.0625 -
                            (src[i-2]+src[i+2]+src[i-w-1]+src[i-w+1]+src[i+w-1]+src[i+w+1])*0.125;

                        if( td < 0 )
                        {
                            td = 0;
                        }
                        else if( td > 255 )
                        {
                            td = 255;
                        }

                        dst[y * w * 3 + x * 3 + 0] = td;
                        dst[y * w * 3 + x * 3 + 1] = src[i];

                        //
                        td = (src[i-w]+src[i+w])*0.5 + src[i]*0.625 + (src[i-2]+src[i+2])*0.0625 -
                            (src[i-2*w]+src[i+2*w]+src[i-w-1]+src[i-w+1]+src[i+w-1]+src[i+w+1])*0.125;
                        if( td < 0 )
                        {
                            td = 0;
                        }
                        else if( td > 255 )
                        {
                            td = 255;
                        }

                        dst[y * w * 3 + x * 3 + 2] = td;

                    }
                }
                else
                {
                    if( x%2 == 1 )
                    {
                        td =  (src[i-w]+src[i+w])*0.5 + src[i]*0.625 + (src[i-2]+src[i+2])*0.0625 -
                            (src[i-2*w]+src[i+2*w]+src[i-w-1]+src[i-w+1]+src[i+w-1]+src[i+w+1])*0.125;
                        if( td < 0 )
                        {
                            td = 0;
                        }
                        else if( td > 255 )
                        {
                            td = 255;
                        }

                        dst[ y * w * 3 + x * 3 + 0 ] = td;
                        dst[ y * w * 3 + x * 3 + 1 ] = src[i];

                        td = (src[i-1]+src[i+1])*0.5 + src[i]*0.625 + (src[i-2*w]+src[i+2*w])*0.0625 -
                            (src[i-2]+src[i+2]+src[i-w-1]+src[i-w+1]+src[i+w-1]+src[i+w+1])*0.125;
                        if( td < 0 )
                        {
                            td = 0;
                        }
                        else if( td > 255 )
                        {
                            td = 255;
                        }

                        dst[ y * w * 3 + x * 3 + 2 ] = td;
                    }
                    else
                    {
                        td  = (src[i-w-1]+src[i-w+1]+src[i+w-1]+src[i+w+1])*0.25 + src[i]*0.75 -
                            (src[i-2*w]+src[i+2*w]+src[i-2]+src[i+2])*0.1875;

                        if( td < 0 )
                        {
                            td = 0;
                        }
                        else if( td > 255 )
                        {
                            td = 255;
                        }

                        dst[ y * w * 3 + x * 3 + 0 ] = td;

                        td = (src[i-1]+src[i+1]+src[i-w]+src[i+w])*0.25 + src[i]*0.5 -
                            (src[i-2*w]+src[i+2*w]+src[i-2]+src[i+2])*0.125;

                        if( td < 0 )
                        {
                            td = 0;
                        }
                        else if( td > 255 )
                        {
                            td = 255;
                        }

                        dst[ y * w * 3 + x * 3 + 1 ] = td;
                        dst[ y * w * 3 + x * 3 + 2 ] = src[i];

                    }
                }
            }
        }
    }
}

void ImageProc::rawToRgb24(const uchar *src, uchar* dst, int w, int h, uchar fmt)
{
    if (!src) return;
    if (!dst) return;

    switch(fmt)
    {
    case IMAGE_MODE_CrYCbY_BG_GR:
        rawToRgb24ForBGGR(src, dst, w, h);
        break;
    case IMAGE_MODE_YCrYCb_GR_BG:
        rawToRgb24ForGRBG(src, dst, w, h);
        break;
    case IMAGE_MODE_YCbYCr_RG_GB:
        rawToRgb24ForRGGB(src, dst, w, h);
        break;
    case IMAGE_MODE_CbYCrY_GB_RG:
        rawToRgb24ForGBRG(src, dst, w, h);
        break;
    }
}

void ImageProc::rawToRgb24ForBGGR(const uchar *src, uchar *dst, int w, int h)
{
    if (!src) return;
    if (!dst) return;
    for(int i=1; i<h-1; i+=2)
    {
        for (int j=1; j<w-1; j+=2)
        {
            //R
            dst[(i*w+j)*3  ] = (src[(i-1)*w + j-1] + src[(i-1)*w + j+1] + src[(i+1)*w + j-1] + src[(i+1)*w + j+1])/4;
            dst[(i*w+j)*3+1] = (src[(i-1)*w + j  ] + src[(i+1)*w + j  ] + src[(i  )*w + j-1] + src[(i  )*w + j+1])/4;
            dst[(i*w+j)*3+2] = src[ (i  )*w + j  ];
            //GR
            dst[(i*w+j)*3+3] = (src[(i-1)*w + j+1] + src[(i+1)*w + j+1])/2;
            dst[(i*w+j)*3+4] = (src[(i  )*w + j+1] + src[(i-1)*w + j+2])/2;//DT
            dst[(i*w+j)*3+5] = (src[(i  )*w + j  ] + src[(i  )*w + j+2])/2;
            //GB
            dst[((i+1)*w+j)*3  ] = (src[(i+1)*w + j-1] + src[(i+1)*w + j+1])/2;
            dst[((i+1)*w+j)*3+1] = (src[(i+1)*w + j  ] + src[(i  )*w + j+1])/2;//DT
            dst[((i+1)*w+j)*3+2] = (src[(i  )*w + j  ] + src[(i+2)*w + j  ])/2;
            //B
            dst[((i+1)*w+j)*3+3] = src[(i+1)*w + j+1];
            dst[((i+1)*w+j)*3+4] = (src[(i  )*w + j+1] + src[(i+2)*w + j+1] + src[(i+1)*w + j] + src[(i+1)*w + j+2])/4;
            dst[((i+1)*w+j)*3+5] = (src[(i  )*w + j  ] + src[(i  )*w + j+2] + src[(i+2)*w + j] + src[(i+2)*w + j+2])/4;
        }
    }

    //边缘插值
    for(int j=1;j<w-1;j+=2)
    {
        int d1=((h-1)*w+j)*3;
        //Gb
        dst[j*3]   = (src[j-1] + src[j+1])/2;
        dst[j*3+1] =  src[j  ];
        dst[j*3+2] =  src[w+j];
        //B
        dst[j*3+3] =  src[      j+1];
        dst[j*3+4] = (src[w+j+1] + src[      j  ] + src[j+2])/3;
        dst[j*3+5] = (src[w+j  ] + src[w+j+2])/2;
        //R
        dst[d1]   = (src[(h-2)*w + j-1] + src[(h-2)*w + j+1])/2;
        dst[d1+1] = (src[(h-2)*w + j  ] + src[(h-1)*w + j+1] + src[(h-1)*w + j-1])/3;
        dst[d1+2] =  src[(h-1)*w + j  ];
        //Gr
        dst[d1+3] =  src[(h-2)*w + j+1];
        dst[d1+4] =  src[(h-1)*w + j+1];
        dst[d1+5] = (src[(h-1)*w + j]+src[(h-1)*w+j+2])/2;

    }

    for(int i=1;i<h-1;i+=2)
    {
        //Gb
        dst[i*w*3]   = (src[(i-1)*w]+src[(i+1)*w])/2;
        dst[i*w*3+1] =  src[i*w];
        dst[i*w*3+2] =  src[i*w+1];
        //B
        dst[(i+1)*w*3]   =  src[(i+1)*w];
        dst[(i+1)*w*3+1] = (src[i*w] + src[(i+2)*w] + src[(i+1)*w+1])/3;
        dst[(i+1)*w*3+2] = (src[i*w+1] + src[(i+2)*w+1])/2;
        //R
        dst[((i+1)*w-1)*3]   = (src[(i )*w-2] + src[(i+2)*w-2])/2;
        dst[((i+1)*w-1)*3+1] = (src[(i )*w-1] + src[(i+2)*w-1] + src[(i+1)*w-2])/3;
        dst[((i+1)*w-1)*3+2] =  src[(i+1)*w-1];
        //Gr
        dst[((i+2)*w-1)*3]   =  src[(i+2)*w-2];
        dst[((i+2)*w-1)*3+1] =  src[(i+2)*w-1];
        dst[((i+2)*w-1)*3+2] = (src[(i+1)*w-1] + src[(i+3)*w-1])/2;
    }
    //B
    dst[0] =  src[0];
    dst[1] = (src[w] + src[1])/2;
    dst[2] =  src[w+1];
    //GB
    dst[3*(w-1)]   = src[w-2];
    dst[3*(w-1)+1] = src[w-1];
    dst[3*(w-1)+2] = src[w-1+w];
    //GR
    dst[(h-1)*3*(w)]   = src[(h-2)*w];
    dst[(h-1)*3*(w)+1] = src[(h-1)*w];
    dst[(h-1)*3*(w)+2] = src[(h-1)*w+1];
    //R
    dst[(h*w-1)*3]   =  src[(h-1)*w-1-1];
    dst[(h*w-1)*3+1] = (src[h*w-1-1] + src[(h-1)*w-1])/2;
    dst[(h*w-1)*3+2] =  src[h*w-1];
}

void ImageProc::rawToRgb24ForRGGB(const uchar* src, uchar *dst, int w, int h)
{
    if (!src) return;
    if (!dst) return;
    for(int i=1;i<h-1;i+=2)
    {
        for (int j=1;j<w-1;j+=2)
        {
            //B
            dst[(i*w+j)*3]   =  src[(i  )*w+j  ];
            dst[(i*w+j)*3+1] = (src[(i-1)*w+j  ] + src[(i+1)*w + j  ] + src[(i  )*w+j-1] + src[(i  )*w + j+1])/4;
            dst[(i*w+j)*3+2] = (src[(i-1)*w+j-1] + src[(i-1)*w + j+1] + src[(i+1)*w+j-1] + src[(i+1)*w + j+1])/4;
            //GB
            dst[(i*w+j)*3+3] = (src[(i  )*w + j  ] + src[(i  )*w + j+2])/2;
            dst[(i*w+j)*3+4] = (src[(i  )*w + j+1] + src[(i-1)*w + j+2])/2;//DT
            dst[(i*w+j)*3+5] = (src[(i-1)*w + j+1] + src[(i+1)*w + j+1])/2;
            //GR
            dst[((i+1)*w+j)*3]   = (src[(i  )*w + j  ] + src[(i+2)*w + j  ])/2;
            dst[((i+1)*w+j)*3+1] = (src[(i+1)*w + j  ] + src[(i  )*w + j+1])/2;//DT
            dst[((i+1)*w+j)*3+2] = (src[(i+1)*w + j-1] + src[(i+1)*w + j+1])/2;
            //R
            dst[((i+1)*w+j)*3+3] = (src[(i  )*w + j  ] + src[(i  )*w+j+2] + src[(i+2)*w + j] + src[(i+2)*w + j+2])/4;
            dst[((i+1)*w+j)*3+4] = (src[(i  )*w + j+1] + src[(i+2)*w+j+1] + src[(i+1)*w + j] + src[(i+1)*w + j+2])/4;
            dst[((i+1)*w+j)*3+5] =  src[(i+1)*w + j+1];
        }
    }

    //边缘插值
    for(int j=1;j<w-1;j+=2)
    {
        int d1 = ((h-1)*w+j)*3;
        //Gr
        dst[j*3]   =  src[w+j];
        dst[j*3+1] =  src[j];
        dst[j*3+2] = (src[j-1] + src[j+1])/2;
        //R
        dst[j*3+3] = (src[w+j  ] + src[w+j+2])/2;
        dst[j*3+4] = (src[w+j+1] + src[j]+ src[j+2])/3;
        dst[j*3+5] =  src[j+1];
        //B
        dst[d1]   =  src[(h-1)*w+j];
        dst[d1+1] = (src[(h-2)*w+j  ] + src[(h-1)*w+j+1] + src[(h-1)*w+j-1])/3;
        dst[d1+2] = (src[(h-2)*w+j-1] + src[(h-2)*w+j+1])/2;
        //Gb
        dst[d1+3] = (src[(h-1)*w+j] + src[(h-1)*w+j+2])/2;
        dst[d1+4] =  src[(h-1)*w+j+1];
        dst[d1+5] =  src[(h-2)*w+j+1];

    }

    for(int i=1;i<h-1;i+=2)
    {
        //Gr
        dst[i*w*3]   =  src[i*w+1];
        dst[i*w*3+1] =  src[i*w];
        dst[i*w*3+2] = (src[(i-1)*w]+src[(i+1)*w])/2;
        //R
        dst[(i+1)*w*3]   = (src[i*w+1] + src[(i+2)*w+1])/2;
        dst[(i+1)*w*3+1] = (src[i*w  ] + src[(i+2)*w  ] + src[(i+1)*w+1])/3;
        dst[(i+1)*w*3+2] =  src[(i+1)*w];
        //B
        dst[((i+1)*w-1)*3]   =  src[(i+1)*w-1];
        dst[((i+1)*w-1)*3+1] = (src[(i)*w-1] + src[(i+2)*w-1] + src[(i+1)*w-2])/3;
        dst[((i+1)*w-1)*3+2] = (src[(i)*w-2] + src[(i+2)*w-2])/2;
        //GB
        dst[((i+2)*w-1)*3]   = (src[(i+1)*w-1] + src[(i+3)*w-1])/2;
        dst[((i+2)*w-1)*3+1] =  src[(i+2)*w-1];
        dst[((i+2)*w-1)*3+2] =  src[(i+2)*w-2];
    }
    //R
    dst[0] = src[w+1];
    dst[1] = (src[w]+src[1])/2;
    dst[2] = src[0];
    //GR
    dst[3*(w-1)]   = src[w-1+w];
    dst[3*(w-1)+1] = src[w-1];
    dst[3*(w-1)+2] = src[w-2];
    //GB
    dst[(h-1)*3*(w)]   = src[(h-1)*w+1];
    dst[(h-1)*3*(w)+1] = src[(h-1)*w];
    dst[(h-1)*3*(w)+2] = src[(h-2)*w];
    //B
    dst[(h*w-1)*3]   =  src[h*w-1];
    dst[(h*w-1)*3+1] = (src[h*w-1-1] + src[(h-1)*w-1])/2;
    dst[(h*w-1)*3+2] =  src[(h-1)*w-1-1];
}

void ImageProc::rawToRgb24ForGRBG(const uchar *src, uchar *dst, int w, int h)
{
    if (!src) return;
    if (!dst) return;
    for(int i=1;i<h-1;i+=2)
    {
        for (int j=1;j<w-1;j+=2)
        {
            //GB
            dst[(i*w+j)*3]   =  (src[(i  )*w + j-1] + src[(i  )*w + j+1])/2;
            dst[(i*w+j)*3+1] =  (src[(i  )*w + j  ] + src[(i-1)*w + j+1])/2; //DT
            dst[(i*w+j)*3+2] =  (src[(i-1)*w + j  ] + src[(i+1)*w + j  ])/2;

            //B
            dst[(i*w+j)*3+3] =   src[(i  )*w + j+1];
            dst[(i*w+j)*3+4] =  (src[(i-1)*w + j+1] + src[(i+1)*w + j+1] + src[(i  )*w+j] + src[(i  )*w + j+2])/4;
            dst[(i*w+j)*3+5] =  (src[(i-1)*w + j  ] + src[(i-1)*w + j+2] + src[(i+1)*w+j] + src[(i+1)*w + j+2])/4;

            //R
            dst[((i+1)*w+j)*3]   =  (src[(i  )*w + j-1] + src[(i  )*w + j+1] + src[(i+2)*w+j-1] + src[(i+2)*w+j+1])/4;
            dst[((i+1)*w+j)*3+1] =  (src[(i  )*w + j  ] + src[(i+2)*w + j  ] + src[(i+1)*w+j-1] + src[(i+1)*w+j+1])/4;
            dst[((i+1)*w+j)*3+2] =   src[(i+1)*w + j  ];

            //GR
            dst[((i+1)*w+j)*3+3] =  (src[(i  )*w + j+1] + src[(i+2)*w + j+1])/2;
            dst[((i+1)*w+j)*3+4] =  (src[(i+1)*w + j+1] + src[(i  )*w + j+2])/2;//DT
            dst[((i+1)*w+j)*3+5] =  (src[(i+1)*w + j  ] + src[(i+1)*w + j+2])/2;

        }
    }

    //边缘插值 上下
    for(int j=1;j<w-1;j+=2)
    {
        int d1 = ((h-1)*w+j)*3;//最后一行

        //R
        dst[j*3]   =  (src[w+j-1]+ src[w+j+1])/2;
        dst[j*3+1] =  (src[w+j ] + src[j-1] + src[j+1])/3;
        dst[j*3+2] =   src[j];
        //Gr
        dst[j*3+3] =   src[w+j+1];
        dst[j*3+4] =   src[j+1];
        dst[j*3+5] =  (src[j] + src[j+2])/2;
        //Gb
        dst[d1]   =  (src[(h-1)*w + j-1] + src[(h-1)*w+j+1])/2;
        dst[d1+1] =   src[(h-1)*w + j];
        dst[d1+2] =   src[(h-2)*w + j];
        //B
        dst[d1+3] =   src[(h-1)*w + j+1];
        dst[d1+4] =  (src[(h-2)*w + j+1] + src[(h-1)*w + j+2] + src[(h-1)*w+j])/3;
        dst[d1+5] =  (src[(h-2)*w + j]   + src[(h-2)*w + j+2])/2;

    }
    //左右
    for(int i=1;i<h-1;i+=2)
    {
        //B
        dst[(i)*w*3]   =  src[(i  )*w];
        dst[(i)*w*3+1] = (src[(i-1)*w  ] + src[(i+1)*w] + src[(i)*w+1])/3;
        dst[(i)*w*3+2] = (src[(i-1)*w+1] + src[(i+1)*w+1])/2;
        //Gb
        dst[(i+1)*w*3]   = (src[(i  )*w  ] + src[(i+2)*w])/2;
        dst[(i+1)*w*3+1] =  src[(i+1)*w  ];
        dst[(i+1)*w*3+2] =  src[(i+1)*w+1];
        //Gr
        dst[((i+1)*w-1)*3]   =  src[(i+1)*w-2];
        dst[((i+1)*w-1)*3+1] =  src[(i+1)*w-1];
        dst[((i+1)*w-1)*3+2] = (src[(i  )*w-1] + src[(i+2)*w-1])/2;
        //R
        dst[((i+2)*w-1)*3] =   (src[(i+1)*w-2] + src[(i+3)*w-2])/2;
        dst[((i+2)*w-1)*3+1] = (src[(i+1)*w-1] + src[(i+3)*w-1] + src[(i+2)*w-2])/3;
        dst[((i+2)*w-1)*3+2] =  src[(i+2)*w-1];

    }
    //Gr
    dst[0] = src[w];
    dst[1] = src[0];
    dst[2] = src[1];
    //R
    dst[3*(w-1)] =    src[2*w-2];
    dst[3*(w-1)+1] = (src[w-2] + src[2*w-1])/2;
    dst[3*(w-1)+2] =  src[w-1];
    //B
    dst[(h-1)*(w)*3] =    src[(h-1)*w];
    dst[(h-1)*(w)*3+1] = (src[(h-1)*w+1] + src[(h-2)*w])/2;
    dst[(h-1)*(w)*3+2] =  src[(h-2)*w+1];
    //Gb
    dst[(h*w-1)*3] =   src[h*w-2];
    dst[(h*w-1)*3+1] = src[h*w-1];
    dst[(h*w-1)*3+2] = src[(h-1)*w-1];
}

void ImageProc::rawToRgb24ForGBRG(const uchar* src, uchar *dst, int w, int h)
{
    if (!src) return;
    if (!dst) return;
    for(int i=1;i<h-1;i+=2)
    {
        for (int j=1;j<w-1;j+=2)
        {
            //Gr
            dst[(i*w+j)*3]   = (src[(i-1)*w + j  ] + src[(i+1)*w + j  ])/2;
            dst[(i*w+j)*3+1] = (src[(i  )*w + j  ] + src[(i-1)*w + j+1])/2; //DT
            dst[(i*w+j)*3+2] = (src[(i  )*w + j-1] + src[(i  )*w + j+1])/2;
            //R
            dst[(i*w+j)*3+3] = (src[(i-1)*w + j  ] + src[(i-1)*w+j+2] + src[(i+1)*w + j] + src[(i+1)*w + j+2])/4;
            dst[(i*w+j)*3+4] = (src[(i-1)*w + j+1] + src[(i+1)*w+j+1] + src[(i  )*w + j] + src[(i  )*w + j+2])/4;
            dst[(i*w+j)*3+5] =  src[(i  )*w + j+1];

            //B
            dst[((i+1)*w+j)*3]   =  src[(i+1)*w + j  ];
            dst[((i+1)*w+j)*3+1] = (src[(i  )*w + j  ] + src[(i+2)*w+j  ] + src[(i+1)*w + j-1] + src[(i+1)*w + j+1])/4;
            dst[((i+1)*w+j)*3+2] = (src[(i  )*w + j-1] + src[(i  )*w+j+1] + src[(i+2)*w + j-1] + src[(i+2)*w + j+1])/4;
            //Gb
            dst[((i+1)*w+j)*3+3] = (src[(i+1)*w + j  ] + src[(i+1)*w + j+2])/2;
            dst[((i+1)*w+j)*3+4] = (src[(i+1)*w + j+1] + src[(i  )*w + j+2])/2;//DT
            dst[((i+1)*w+j)*3+5] = (src[(i  )*w + j+1] + src[(i+2)*w + j+1])/2;

        }
    }

    //边缘插值 上下
    for(int j=1;j<w-1;j+=2)
    {
        int d1 = ((h-1)*w+j)*3;//最后一行

        //B
        dst[j*3] =    src[j];
        dst[j*3+1] = (src[w+j] + src[j-1] + src[j+1])/3;
        dst[j*3+2] = (src[w+j-1] + src[w+j+1])/2;
        //Gb
        dst[j*3+3] = (src[j] + src[j+2])/2;
        dst[j*3+4] =  src[j+1];
        dst[j*3+5] =  src[w+j+1];
        //Gr
        dst[d1] =    src[(h-2)*w + j];
        dst[d1+1] =  src[(h-1)*w + j];
        dst[d1+2] = (src[(h-1)*w + j-1] + src[(h-1)*w+j+1])/2;
        //R
        dst[d1+3] = (src[(h-2)*w+j  ] + src[(h-2)*w+j+2])/2;
        dst[d1+4] = (src[(h-2)*w+j+1] + src[(h-1)*w+j+2] + src[(h-1)*w+j])/3;
        dst[d1+5] =  src[(h-1)*w+j+1];

    }
    //左右
    for(int i=1;i<h-1;i+=2)
    {
        //R
        dst[(i)*w*3] =   (src[(i-1)*w + 1]+ src[(i+1)*w + 1])/2;
        dst[(i)*w*3+1] = (src[(i-1)*w] + src[(i+1)*w] + src[(i)*w+1])/3;
        dst[(i)*w*3+2] =  src[(i)*w];
        //Gb
        dst[(i+1)*w*3]   =  src[(i+1)*w+1];
        dst[(i+1)*w*3+1] =  src[(i+1)*w];
        dst[(i+1)*w*3+2] = (src[(i  )*w] + src[(i+2)*w])/2;
        //Gr
        dst[((i+1)*w-1)*3]   = (src[(i)*w-1] + src[(i+2)*w-1])/2;
        dst[((i+1)*w-1)*3+1] =  src[(i+1)*w-1];
        dst[((i+1)*w-1)*3+2] =  src[(i+1)*w-2];
        //B
        dst[((i+2)*w-1)*3]   =  src[(i+2)*w-1];
        dst[((i+2)*w-1)*3+1] = (src[(i+1)*w-1] + src[(i+3)*w-1] + src[(i+2)*w-2])/3;
        dst[((i+2)*w-1)*3+2] = (src[(i+1)*w-2] + src[(i+3)*w-2])/2;

    }
    //Gb
    dst[0] = src[1];
    dst[1] = src[0];
    dst[2] = src[w];
    //B
    dst[3*(w-1)]   =  src[w-1];
    dst[3*(w-1)+1] = (src[w-2] + src[2*w-1])/2;
    dst[3*(w-1)+2] =  src[2*w-2];
    //R
    dst[(h-1)*(w)*3]   =  src[(h-2)*w+1];
    dst[(h-1)*(w)*3+1] = (src[(h-1)*w+1] + src[(h-2)*w])/2;
    dst[(h-1)*(w)*3+2] =  src[(h-1)*w];
    //Gr
    dst[(h*w-1)*3]   = src[(h-1)*w-1];
    dst[(h*w-1)*3+1] = src[h*w-1];
    dst[(h*w-1)*3+2] = src[h*w-2];
}


void ImageProc::rgb24ToRaw(const uchar *src, uchar *dst, int w, int h, uchar fmt)
{
    if (!src) return;
    if (!dst) return;

    switch(fmt)
    {
    case IMAGE_MODE_CrYCbY_BG_GR:
        rgb24ToRawForBGGR(src, dst, w, h);
        break;
    case IMAGE_MODE_YCrYCb_GR_BG:
        rgb24ToRawForGRBG(src, dst, w, h);
        break;
    case IMAGE_MODE_YCbYCr_RG_GB:
        rgb24ToRawForRGGB(src, dst, w, h);
        break;
    case IMAGE_MODE_CbYCrY_GB_RG:
        rgb24ToRawForGBRG(src, dst, w, h);
        break;
    }
}

void ImageProc::rgb24ToRawForBGGR(const uchar * src, uchar *dst, int w, int h)
{
    if (!src) return;
    if (!dst) return;
    for (int i = 0; i < h; i += 2)
    {
        for (int j = 0; j < w; j += 2)
        {
            //B
            dst[i*w + j] = src[i*w*3 + j*3];
            //Gb
            dst[i*w + (j+1)] = src[i*w*3 + (j+1)*3 + 1];
            //Gr
            dst[(i+1)*w + j] = src[(i+1)*w*3 + j*3 + 1];
            //R
            dst[(i+1)*w +(j+1)] = src[(i+1)*w*3 + (j+1)*3 + 2];
        }
    }
}

void ImageProc::rgb24ToRawForGRBG(const uchar * src, uchar *dst, int w, int h)
{
    if (!src) return;
    if (!dst) return;
    for (int i = 0; i < h; i += 2)
    {
        for (int j = 0; j < w; j+=2)
        {
            //Gr
            dst[i*w + j] = src[i*w*3 + j*3 + 1];
            //R
            dst[i*w + (j+1)] = src[i*w*3 + (j+1)*3 + 2];
            //B
            dst[(i+1)*w + j] = src[(i+1)*w*3 +j*3];
            //Gb
            dst[(i+1)*w +(j+1)] = src[(i+1)*w*3 + (j+1)*3 + 1];
        }
    }
}

void ImageProc::rgb24ToRawForGBRG(const uchar * src, uchar *dst, int w, int h)
{
    if (!src) return;
    if (!dst) return;
    for (int i = 0; i < h; i += 2)
    {
        for (int j = 0; j < w; j += 2)
        {
            //Gb
            dst[i*w + j] = src[i*w*3 + j*3 + 1];
            //B
            dst[i*w + (j+1)] = src[i*w*3 + (j+1)*3];
            //R
            dst[(i+1)*w + j] = src[(i+1)*w*3 +j*3 + 2];
            //Gr
            dst[(i+1)*w +(j+1)] = src[(i+1)*w*3 + (j+1)*3 + 1];
        }
    }
}

void ImageProc::rgb24ToRawForRGGB(const uchar * src, uchar *dst, int w, int h)
{
    if (!src) return;
    if (!dst) return;
    for (int i = 0; i < h; i += 2)
    {
        for (int j = 0; j < w; j += 2)
        {
            //R
            dst[i*w + j] = src[i*w*3 + j*3 + 2];
            //Gr
            dst[i*w + (j+1)] = src[i*w*3 + (j+1)*3 + 1];
            //Gb
            dst[(i+1)*w + j] = src[(i+1)*w*3 +j*3 + 1];
            //B
            dst[(i+1)*w +(j+1)] = src[(i+1)*w*3 + (j+1)*3];
        }
    }
}
