package com.exe;

import java.util.Locale;

import javax.microedition.khronos.opengles.GL11;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;

import android.opengl.GLUtils;

public class eri
{  
  public static int CreateTxtTexture(Activity activity, String txt, String font_name, float font_size, boolean align_center)
  {
    Paint textPaint = new Paint();
    textPaint.setTextSize(font_size);
    textPaint.setTextAlign(align_center ? Paint.Align.CENTER : Paint.Align.LEFT);
    textPaint.setAntiAlias(true);
    textPaint.setARGB(0xff, 0xFF, 0xFF, 0xFF);

    int width = 0;
    int height = 0;

    int line_height = (int) Math.ceil(-textPaint.ascent() + textPaint.descent());

    String[] lines = txt.split("\n");

    for(String line: lines)
    {
      int line_width = (int) Math.ceil(textPaint.measureText(line));
      if (line_width > width)
        width = line_width;

      height += line_height;
    }

    int power2_width = Integer.highestOneBit(width);
    if (power2_width < width) power2_width = power2_width << 1;
    int power2_height = Integer.highestOneBit(height);
    if (power2_height < height) power2_height = power2_height << 1;

    // Log.i("CreateTxtTexture", "calculate w " + width + " h " + height + " power2 w " + power2_width + " h " + power2_height);

    Bitmap bitmap = Bitmap.createBitmap(power2_width, power2_height, Bitmap.Config.ARGB_8888);
    Canvas canvas = new Canvas(bitmap);
    bitmap.eraseColor(0);

    float origin_x = align_center ? (width / 2) : 0;
    float origin_y = -textPaint.ascent();

    for(String line: lines)
    {
      canvas.drawText(line, origin_x, origin_y, textPaint);
      origin_y += line_height;
    }

    // Log.i("CreateTxtTexture", txt + " use " + font_name + " " + font_size);

    // Use the Android GLUtils to specify a two-dimensional texture image from our bitmap
    GLUtils.texImage2D(GL11.GL_TEXTURE_2D, 0, bitmap, 0);

    bitmap.recycle();

    return width + (height << 16);
  }
  
  public static String GetInternalPath(Activity activity)
  {
    return activity.getApplicationContext().getFilesDir().getPath();
  }
  
  public static String GetLocale()
  {
    return Locale.getDefault().getLanguage() + "_" + Locale.getDefault().getCountry();
  }
  
  public static int GetDisplayRotate(Activity activity)
  {
    return activity.getWindow().getWindowManager().getDefaultDisplay().getRotation();
  }
}
