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
            copyAssetFile(name, new File(destination, name));
        }
        copyAssetTree("assets", new File(destination, "assets"));
    }

    private void copyAssetFile(String name, File output) {
        if (output.isFile() && output.length() > 0) {
            return;
        }
        File parent = output.getParentFile();
        if (parent != null) {
            parent.mkdirs();
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

    private void copyAssetTree(String assetPath, File output) {
        try {
            String[] children = getAssets().list(assetPath);
            if (children == null || children.length == 0) {
                copyAssetFile(assetPath, output);
                return;
            }
            output.mkdirs();
            for (String child : children) {
                copyAssetTree(assetPath + "/" + child, new File(output, child));
            }
        } catch (IOException ignored) {
            // Native startup reports missing bootstrap data through its GUI.
        }
    }
}
