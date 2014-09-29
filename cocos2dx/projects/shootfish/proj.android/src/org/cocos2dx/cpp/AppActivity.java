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
import com.google.android.gms.games.Games;
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.example.games.basegameutils.GameHelper;

import com.lxjsky.shootfish.R;

import android.content.Intent;
import android.os.Bundle;
import android.view.ViewGroup;
import android.view.View;
import android.util.Log;
import android.os.Handler;
import android.os.Message;
import android.view.WindowManager;
import android.view.Gravity;

public class AppActivity extends Cocos2dxActivity {

    private static final String TAG = "shootfish";
    private static final int MSG_SHOW_ADS = 100;
    private static final int MSG_HIDE_ADS = 101;
    private static final int MSG_SHOW_ACHIEVENTMENTS = 102;
    private static final int MSG_SHOW_LEADERBOARDS = 103;
    private static AdView mAdView = null;
    private static Handler mHandler = null;
	private static WindowManager mWm = null;
    private static GameHelper mHelper;
    private static AppActivity appActivity;
    private static final String mAdUnitId = "ca-app-pub-6909996343483118/2618996984";

    private void _requestAds() {
        if (mAdView != null)
            _destroyAds();

        mAdView = new AdView(this);
        mAdView.setAdUnitId(mAdUnitId);
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

    private void _destroyAds() {
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

    private void _showAchievements() {
        if (isSignedIn()) {
            startActivityForResult(Games.Achievements.getAchievementsIntent(getApiClient()), 0);
        } else {
            //showAlert(getString(R.string.achievements_not_available));
            beginUserInitiatedSignIn();
        }
    }

    private void _showLeaderboards() {
        if (isSignedIn()) {
            //startActivityForResult(Games.Leaderboards.getAllLeaderboardsIntent(getApiClient()), 0);
            startActivityForResult(Games.Leaderboards.getLeaderboardIntent(getApiClient(), getString(R.string.leaderboard)), 0);
        } else {
            beginUserInitiatedSignIn();
            //showAlert(getString(R.string.leaderboards_not_available));
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        appActivity = this;

        if (null == mWm) {
            mWm = (WindowManager) this.getSystemService("window");
        }

        mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                case MSG_SHOW_ADS:
                    _requestAds();
                    break;
                case MSG_HIDE_ADS:
                    _destroyAds();
                    break;
                case MSG_SHOW_ACHIEVENTMENTS:
                    _showAchievements();
                    break;
                case MSG_SHOW_LEADERBOARDS:
                    _showLeaderboards();
                    break;
                }
            }
        };

        if (mHelper == null) {
            getGameHelper();
        }

        mHelper.setup(new GameHelper.GameHelperListener() {
            @Override
            public void onSignInFailed() {
                Log.d(TAG, "onSignInFailed()");
            }

            @Override
            public void onSignInSucceeded() {
                Log.d(TAG, "onSignInSucceeded()");
            }
        });

    }

    public GameHelper getGameHelper() {
        if (mHelper == null) {
            mHelper = new GameHelper(this, GameHelper.CLIENT_GAMES);
            mHelper.enableDebugLog(true);
            mHelper.setMaxAutoSignInAttempts(0);
        }
        return mHelper;
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

    @Override
    protected void onStart() {
        super.onStart();
        mHelper.onStart(this);
    }

    @Override
    protected void onStop() {
        super.onStop();
        mHelper.onStop();
    }

    @Override
    protected void onActivityResult(int request, int response, Intent data) {
        super.onActivityResult(request, response, data);
        mHelper.onActivityResult(request, response, data);
    }

    protected GoogleApiClient getApiClient() {
        return mHelper.getApiClient();
    }

    protected boolean isSignedIn() {
        return mHelper.isSignedIn();
    }

    protected void beginUserInitiatedSignIn() {
        mHelper.beginUserInitiatedSignIn();
    }

    protected void signOut() {
        mHelper.signOut();
    }

    protected void showAlert(String message) {
        mHelper.makeSimpleDialog(message).show();
    }

    protected void showAlert(String title, String message) {
        mHelper.makeSimpleDialog(title, message).show();
    }

    protected void enableDebugLog(boolean enabled) {
        if (mHelper != null) {
            mHelper.enableDebugLog(enabled);
        }
    }

    protected String getInvitationId() {
        return mHelper.getInvitationId();
    }

    protected void reconnectClient() {
        mHelper.reconnectClient();
    }

    protected boolean hasSignInError() {
        return mHelper.hasSignInError();
    }

    protected GameHelper.SignInFailureReason getSignInError() {
        return mHelper.getSignInError();
    }

    /*
     * for C++
     */
    public static void showAds() {
        mHandler.sendEmptyMessage(MSG_SHOW_ADS);
        Log.d(TAG, "showAds");
    }

    public static void hideAds() {
        mHandler.sendEmptyMessage(MSG_HIDE_ADS);
        Log.d(TAG, "hideAds");
    }

    public static void showAchievements() {
        mHandler.sendEmptyMessage(MSG_SHOW_ACHIEVENTMENTS);
        Log.d(TAG, "show achievements");
    }

    public static void showLeaderboards() {
        mHandler.sendEmptyMessage(MSG_SHOW_LEADERBOARDS);
        Log.d(TAG, "show leaderboards");
    }

    public static void storeLeaderboards(int score) {
        Log.d(TAG, "storeLeaderboards");
        final int curScore = score;
        appActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (appActivity.isSignedIn()) {
                    Games.Leaderboards.submitScore(appActivity.getApiClient(), appActivity.getString(R.string.leaderboard), curScore);
                    Log.d(TAG, "submitScore " + curScore);
                }
            }
        });
    }
}
