package org.oxygine.HelloWorldFacebook;

import org.oxygine.facebook.FacebookAdapter;
import org.oxygine.lib.OxygineActivity;
import android.os.Bundle;

public class MainActivity extends OxygineActivity
{
 	@Override
    protected void onCreate(Bundle savedInstanceState) {
        addObserver(new FacebookAdapter(this));

    	super.onCreate(savedInstanceState);
    }
}
