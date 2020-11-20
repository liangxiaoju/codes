
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Set;

import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.AdSize;
import com.google.android.gms.ads.AdView;
import com.google.android.gms.ads.AdListener;

import android.app.Activity;
import android.content.Context;
import android.util.Log;
import android.view.WindowManager;

public class AdsAdmob {
    private static final String TAG = "AdsAdmob";

    private AdView mAdView = null;
    private Set<String> mTestDevices = null;
    private WindowManager mWm = null;

    protected static Context sContext = null;
    protected static Handler sMainThreadHandler = null;
	private static boolean bDebug = false;

    public static void init(Context context) {
        sContext = context;
        if (null == sMainThreadHandler) {
            sMainThreadHandler = new Handler();
        }
    }

    public static void runOnMainThread(Runnable r) {
        if (null != sMainThreadHandler) {
            sMainThreadHandler.post(r);
        } else
        if (null != sContext && sContext instanceof Activity) {
            Activity act = (Activity) sContext;
            act.runOnUiThread(r);
        }
    }

	protected static void LogE(String msg, Exception e) {
		Log.e(LOG_TAG, msg, e);
		e.printStackTrace();
	}

	protected static void LogD(String msg) {
		if (bDebug) {
			Log.d(LOG_TAG, msg);
		}
	}

    public void setDebugMode(boolean debug) {
        bDebug = debug;
    }

    public void configDeveloperInfo(Hashtable<String, String> devInfo) {

		try {
			mPublishID = devInfo.get("AdmobID");
			LogD("init AppInfo : " + mPublishID);
		} catch (Exception e) {
			LogE("initAppInfo, The format of appInfo is wrong", e);
		}
    }

	public void showAds(Hashtable<String, String> info, int pos) {
	    try
	    {
	        String strType = info.get("AdmobType");
	        int adsType = Integer.parseInt(strType);

	        switch (adsType) {
	        case ADMOB_TYPE_BANNER:
	            {
	                String strSize = info.get("AdmobSizeEnum");
	                int sizeEnum = Integer.parseInt(strSize);
    	            showBannerAd(sizeEnum, pos);
                    break;
	            }
	        case ADMOB_TYPE_FULLSCREEN:
	            LogD("Now not support full screen view in Admob");
	            break;
	        default:
	            break;
	        }
	    } catch (Exception e) {
	        LogE("Error when show Ads ( " + info.toString() + " )", e);
	    }
	}

	public void hideAds(Hashtable<String, String> info) {
	    try
        {
            String strType = info.get("AdmobType");
            int adsType = Integer.parseInt(strType);

            switch (adsType) {
            case ADMOB_TYPE_BANNER:
                hideBannerAd();
                break;
            case ADMOB_TYPE_FULLSCREEN:
                LogD("Now not support full screen view in Admob");
                break;
            default:
                break;
            }
        } catch (Exception e) {
            LogE("Error when hide Ads ( " + info.toString() + " )", e);
        }
	}

	private void showBannerAd(int sizeEnum, int pos) {
		final int curPos = pos;
		final int curSize = sizeEnum;

		PluginWrapper.runOnMainThread(new Runnable() {

			@Override
			public void run() {
				// destory the ad view before
				if (null != adView) {
					if (null != mWm) {
						mWm.removeView(adView);
					}
					adView.destroy();
					adView = null;
				}

				AdSize size = AdSize.BANNER;
				switch (curSize) {
				case AdsAdmob.ADMOB_SIZE_BANNER:
					size = AdSize.BANNER;
					break;
				case AdsAdmob.ADMOB_SIZE_IABMRect:
					size = AdSize.MEDIUM_RECTANGLE;
					break;
				case AdsAdmob.ADMOB_SIZE_IABBanner:
					size = AdSize.FULL_BANNER;
					break;
				case AdsAdmob.ADMOB_SIZE_IABLeaderboard:
					size = AdSize.LEADERBOARD;
					break;
				case AdsAdmob.ADMOB_SIZE_Skyscraper:
				    size = AdSize.WIDE_SKYSCRAPER;
				    break;
				default:
					break;
				}
				adView = new AdView(mContext);
                adView.setAdUnitId(mPublishID);
                adView.setAdSize(size);
				AdRequest.Builder req = new AdRequest.Builder();
				
				try {
					if (mTestDevices != null) {
						Iterator<String> ir = mTestDevices.iterator();
						while(ir.hasNext())
						{
							req.addTestDevice(ir.next());
						}
					}
				} catch (Exception e) {
					LogE("Error during add test device", e);
				}
				
				adView.loadAd(req.build());
				adView.setAdListener(new AdmobAdsListener());

				if (null == mWm) {
					mWm = (WindowManager) mContext.getSystemService("window");
				}
				AdsWrapper.addAdView(mWm, adView, curPos);
			}
		});
	}

	private void hideBannerAd() {
		PluginWrapper.runOnMainThread(new Runnable() {
			@Override
			public void run() {
				if (null != adView) {
					if (null != mWm) {
						mWm.removeView(adView);
					}
					adView.destroy();
					adView = null;
				}
			}
		});
	}

	public void addTestDevice(String deviceID) {
		LogD("addTestDevice invoked : " + deviceID);
		if (null == mTestDevices) {
			mTestDevices = new HashSet<String>();
		}
		mTestDevices.add(deviceID);
	}

	private class AdmobAdsListener extends AdListener {

        @Override
        public void onAdLoaded() {
			LogD("onAdLoaded invoked");
        }

        @Override
        public void onAdFailedToLoad(int errorCode) {
            String errorReason = "";
            switch(errorCode) {
                case AdRequest.ERROR_CODE_INTERNAL_ERROR:
                    errorReason = "Internal error";
                    break;
                case AdRequest.ERROR_CODE_INVALID_REQUEST:
                    errorReason = "Invalid request";
                    break;
                case AdRequest.ERROR_CODE_NETWORK_ERROR:
                    errorReason = "Network Error";
                    break;
                case AdRequest.ERROR_CODE_NO_FILL:
                    errorReason = "No fill";
                    break;
            }
			LogD("failed to receive ad : " + errorCode + " , " + errorReason);
        }

        @Override
        public void onAdOpened() {
			LogD("onAdOpened invoked");
        }

        @Override
        public void onAdClosed() {
			LogD("onAdClosed invoked");
        }

        @Override
        public void onAdLeftApplication() {
			LogD("onAdLeftApplication invoked");
        }
    }

}

