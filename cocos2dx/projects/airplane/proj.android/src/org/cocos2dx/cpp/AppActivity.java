/****************************************************************************
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2011      Zynga Inc.
Copyright (c) 2013-2014 Chukong Technologies Inc.
 
http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
package org.cocos2dx.cpp;

import org.cocos2dx.lib.Cocos2dxActivity;

import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.AdSize;
import com.google.android.gms.ads.AdView;

import android.os.Bundle;
import android.view.ViewGroup;
import android.view.View;
import android.util.Log;
import android.os.Handler;
import android.os.Message;
import android.view.WindowManager;
import android.view.Gravity;

public class AppActivity extends Cocos2dxActivity {

    private static final String TAG = "airplane";
    private static final int AD_SHOW = 100;
    private static final int AD_HIDE = 101;
    private static AdView mAdView = null;
    private static Handler mHandler = null;
	private WindowManager mWm = null;

    private void requestAd() {
        if (mAdView != null)
            destroyAd();

        mAdView = new AdView(this);
        mAdView.setAdUnitId("ca-app-pub-6909996343483118/5291005782");
        mAdView.setAdSize(AdSize.BANNER);
        mAdView.setAdListener(new ToastAdListener(this) {
            @Override
            public void onAdLoaded() {
                super.onAdLoaded();
                showAdView();
            }
        });

        AdRequest adReq = new AdRequest.Builder()
//            .addTestDevice("F9CC34C4B0F028BACBB4CAF80A8548BB")
            .build();
        mAdView.loadAd(adReq);
    }

    private void destroyAd() {
        if (mAdView != null) {
            if (mWm != null) {
                try {
                    mWm.removeView(mAdView);
                } catch (IllegalArgumentException e) {
                    ;
                }
            }
            mAdView.destroy();
            mAdView = null;
        }
    }

    private void showAdView() {
        WindowManager.LayoutParams params = new WindowManager.LayoutParams();
        params.type = WindowManager.LayoutParams.TYPE_APPLICATION_ATTACHED_DIALOG;
        params.width = WindowManager.LayoutParams.WRAP_CONTENT;
        params.height = WindowManager.LayoutParams.WRAP_CONTENT;
        params.flags |= WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE;
        params.gravity = Gravity.TOP | Gravity.CENTER;
        try {
            mWm.removeView(mAdView);
        } catch (IllegalArgumentException e) {
            ;
        }
        mWm.addView(mAdView, params);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (null == mWm) {
            mWm = (WindowManager) this.getSystemService("window");
        }

        mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                case AD_SHOW:
                    requestAd();
                    break;
                case AD_HIDE:
                    destroyAd();
                    break;
                }
            }
        };
    }

    public static void showAd(boolean on) {
        mHandler.sendEmptyMessage(on ? AD_SHOW: AD_HIDE);

        Log.d(TAG, "Ad " + (on ? "show" : "hide"));
    }

    @Override
    protected void onPause() {
        if (mAdView != null)
            mAdView.pause();
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mAdView != null)
            mAdView.resume();
    }

    @Override
    protected void onDestroy() {
        if (mAdView != null)
            mAdView.destroy();
        super.onDestroy();
    }
}
