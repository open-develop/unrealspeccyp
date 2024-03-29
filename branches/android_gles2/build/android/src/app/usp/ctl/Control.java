/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2011 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

package app.usp.ctl;

import android.content.Context;
import android.content.res.Configuration;
import android.view.KeyEvent;
import android.widget.ImageView;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import app.usp.Emulator;
import app.usp.Preferences;
import app.usp.R;
import app.usp.ViewGLES;

public class Control extends ImageView
{
	private ControlSensor sensor;
    private Bitmap keyboard;
    private Bitmap joystick;
    private boolean keyboard_active = false;
    private ViewGLES view = null;

	public Control(Context context, ViewGLES _view)
	{
		super(context);
		view = _view;

		keyboard = BitmapFactory.decodeResource(getResources(), R.drawable.keyboard);
		joystick = BitmapFactory.decodeResource(getResources(), R.drawable.joystick);
		keyboard_active = Emulator.the.GetOptionBool(Preferences.use_keyboard_id);
		sensor = new ControlSensor(context);
		setAdjustViewBounds(true);
		setFocusable(true);
		setFocusableInTouchMode(true);
		setImageBitmap(keyboard_active ? keyboard : joystick);
		setOnKeyListener(new ControlKeys());
		setOnTouchListener(new ControlTouch() {
			public void OnTouch(float x, float y, boolean down, int pid)
			{
				Emulator.the.OnTouch(keyboard_active, x/getWidth(), y/getHeight(), down, pid);
			}}
		);
	}
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		if(keyCode == KeyEvent.KEYCODE_BACK)
		{
			keyboard_active = !keyboard_active;
			Emulator.the.SetOptionBool(Preferences.use_keyboard_id, keyboard_active);
			setImageBitmap(keyboard_active ? keyboard : joystick);
			view.OnControlsToggle();
			return true;
		}
		return false;
	}
	protected void onMeasure(int w, int h)
	{
		if(getResources().getConfiguration().orientation == Configuration.ORIENTATION_LANDSCAPE)
			setMeasuredDimension(0, 0);
		else
			super.onMeasure(w, h);
	}
	public void OnResume()	{ sensor.Install(); }
	public void OnPause()	{ sensor.Uninstall(); }
	
	public static void UpdateJoystickKeys(final boolean left, final boolean right, final boolean up, final boolean down)
	{
		// release keys first
		if(!left)
			Emulator.the.OnKey('l', false, false, false);
		if(!right)
			Emulator.the.OnKey('r', false, false, false);
		if(!up)
			Emulator.the.OnKey('u', false, false, false);
		if(!down)
			Emulator.the.OnKey('d', false, false, false);

		// then press keys
		if(left)
			Emulator.the.OnKey('l', true, false, false);
		if(right)
			Emulator.the.OnKey('r', true, false, false);
		if(up)
			Emulator.the.OnKey('u', true, false, false);
		if(down)
			Emulator.the.OnKey('d', true, false, false);
	}
}
