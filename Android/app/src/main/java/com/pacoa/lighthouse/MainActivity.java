package com.pacoa.lighthouse;

import android.os.Bundle;
import android.view.WindowManager;
import org.libsdl.app.SDLActivity;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

/** SDL launcher for the native Lighthouse library.
 *
 * Game data stays in app-specific external storage: no broad storage permission
 * is requested, while adb and Android's file picker can still place a legally
 * dumped baserom.us.z64 alongside the generated bk.o2r archive.
 */
public final class MainActivity extends SDLActivity {
    private static final String[] BOOTSTRAP_ASSETS = { "lighthouse.o2r", "config.yml" };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        bootstrapAssets();
        super.onCreate(savedInstanceState);
    }

    @Override
    protected String[] getLibraries() {
        return new String[] { "Lighthouse" };
    }

    private void bootstrapAssets() {
        File destination = getExternalFilesDir(null);
        if (destination == null) {
            return;
        }
        for (String name : BOOTSTRAP_ASSETS) {
            File output = new File(destination, name);
            if (output.isFile() && output.length() > 0) {
                continue;
            }
            try (InputStream input = getAssets().open(name);
                 FileOutputStream stream = new FileOutputStream(output)) {
                byte[] buffer = new byte[32 * 1024];
                int count;
                while ((count = input.read(buffer)) != -1) {
                    stream.write(buffer, 0, count);
                }
            } catch (IOException ignored) {
                // Native startup reports missing bootstrap data through its GUI.
            }
        }
    }
}
