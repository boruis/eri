package com.exe;

import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

import javax.microedition.khronos.opengles.GL11;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Typeface;

import android.opengl.GLUtils;

public class eri {

  private static final String TAG = "eri";

  private static Map<String, Typeface> typeface_map = null;

  public static int CreateSysTxtTexture(
      Activity activity,
      String txt,
      String font_name,
      float font_size,
      boolean align_center) {

    if (null == typeface_map)
      typeface_map = new HashMap<>();

    Typeface typeface = typeface_map.get(font_name);

    if (null == typeface && font_name.endsWith(".ttf")) {
      typeface = Typeface.createFromAsset(activity.getAssets(), font_name);
      if (null != typeface)
        typeface_map.put(font_name, typeface);
    }

    if (null == typeface) {
      Typeface family = Typeface.DEFAULT;
      int style = Typeface.NORMAL;

      if (!font_name.isEmpty()) {
        String[] parts = font_name.split("\\.");

        if (parts.length > 0 && !parts[0].isEmpty()) {
          if (parts[0].compareTo("MONOSPACE") == 0)
            family = Typeface.MONOSPACE;
          else if (parts[0].compareTo("SANS_SERIF") == 0)
            family = Typeface.SANS_SERIF;
          else if (parts[0].compareTo("SERIF") == 0)
            family = Typeface.SERIF;
        }

        if (parts.length > 1 && !parts[1].isEmpty()) {
          if (parts[1].compareTo("BOLD") == 0)
            style = Typeface.BOLD;
          else if (parts[1].compareTo("BOLD_ITALIC") == 0)
            style = Typeface.BOLD_ITALIC;
          else if (parts[1].compareTo("ITALIC") == 0)
            style = Typeface.ITALIC;
        }
      }

      if (Typeface.DEFAULT == family && Typeface.BOLD == style) {
        typeface = Typeface.DEFAULT_BOLD;
      }
      else if (Typeface.NORMAL == style) {
        typeface = family;
      }
      else {
        typeface = Typeface.create(family, style);
        if (null != typeface)
          typeface_map.put(font_name, typeface);
      }
    }

    return CreateTxtTexture(txt, typeface, font_size, align_center);
  }

  static int CreateTxtTexture(
      String txt,
      Typeface typeface,
      float font_size,
      boolean align_center) {

    Paint textPaint = new Paint();
    textPaint.setTypeface(typeface);
    textPaint.setTextSize(font_size);
    textPaint.setTextAlign(align_center ? Paint.Align.CENTER : Paint.Align.LEFT);
    textPaint.setAntiAlias(true);
    textPaint.setARGB(0xff, 0xFF, 0xFF, 0xFF);

    int width = 0;
    int height = 0;

    int line_height = (int)Math.ceil(-textPaint.ascent() + textPaint.descent());

    String[] lines = txt.split("\n");

    for (String line : lines) {
      int line_width = (int)Math.ceil(textPaint.measureText(line));
      if (line_width > width)
        width = line_width;

      height += line_height;
    }

    int power2_width = Integer.highestOneBit(width);
    if (power2_width < width)
      power2_width = power2_width << 1;
    int power2_height = Integer.highestOneBit(height);
    if (power2_height < height)
      power2_height = power2_height << 1;

    // Log.i(TAG, "CreateTxtTexture calculate w " + width + " h " + height +
    // " power2 w " + power2_width + " h " + power2_height);

    if (0 == power2_width || 0 == power2_height)
      return 0;

    Bitmap bitmap = Bitmap.createBitmap(power2_width, power2_height, Bitmap.Config.ARGB_8888);
    Canvas canvas = new Canvas(bitmap);
    bitmap.eraseColor(0);

    float origin_x = align_center ? (width / 2) : 0;
    float origin_y = -textPaint.ascent();

    for (String line : lines) {
      canvas.drawText(line, origin_x, origin_y, textPaint);
      origin_y += line_height;
    }

    // canvas.setBitmap(null);

    // Log.i(TAG, "CreateTxtTexture " + txt + " use " + font_name + " " + font_size);

    // Use the Android GLUtils to specify a two-dimensional texture image
    // from our bitmap
    GLUtils.texImage2D(GL11.GL_TEXTURE_2D, 0, bitmap, 0);

    // bitmap.recycle();

    return width + (height << 16);
  }

  public static String GetInternalPath(Activity activity) {
    return activity.getApplicationContext().getFilesDir().getPath();
  }

  public static String GetLocale() {
    return Locale.getDefault().getLanguage() + "_" + Locale.getDefault().getCountry();
  }

  public static int GetDisplayRotate(Activity activity) {
    return activity.getWindow().getWindowManager().getDefaultDisplay().getRotation();
  }
}
