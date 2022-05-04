\page gui_config GUI Configuration

Ignition Gazebo's graphical user interface is powered by
[Ignition GUI](https://ignitionrobotics.org/libs/gui). Therefore, Gazebo's
GUI layout can be defined in
[Ignition GUI configuration files](https://ignitionrobotics.org/api/gui/2.1/config.html).
These are XML files that describe what plugins to be loaded and with what
settings.

## How to load different GUI configurations

There are a few places where the GUI configuration can come from:

1. A file passed to the `--gui-config` command line argument
2. A `<gui>` element inside an SDF file
3. The default configuration file at `$HOME/.ignition/gazebo/gui.config` \*

Each of the items above takes precedence over the ones below it by default.
For example, if a user chooses a `--gui-config`, the SDF's `<gui>` element is
ignored. The default configuration file is loaded if no configuration is
passed through the command line or the SDF file.

The default behaviour can be overridden using the `--gui-config-option`
command line argument, which offers options:

* `prepend`: Load plugins from both the config file and the SDF file. The config
  file is loaded first.
* `merge`: Like `prepend`, but only the last instance of each plugin is loaded,
  which should usually come from the SDF file.

The table below summarizes how all the options behave together.

Not all
combinations are useful, but we cover all corner cases (i.e. there's no point
in providing a config and also opting to `ignore` it):

--gui-config | SDF `<gui>`  | --gui-config-option     | Result
------------ | ------------ | ----------------------- | ------
empty        | no plugins   | empty / prepend / merge | Load plugins from `$HOME/.ignition/gazebo/gui.config`
provided     | no plugins   | empty / prepend / merge | Load plugins from `--gui-config`
empty        | has plugins  | empty                   | Load plugins from SDF
empty        | has plugins  | prepend / merge         | Load plugins from `$HOME/.ignition/gazebo/gui.config` then SDF, merging as needed
provided     | has plugins  | empty                   | Load plugins from `--gui-config`
provided     | has plugins  | prepend / merge         | Load plugins from `--gui-config` then SDF, merging as needed

> For log-playback, the default file is
> `$HOME/.ignition/gazebo/playback_gui.config`

## Try it out

### Default configuration

Let's try this in practice. First, let's open Ignition Gazebo without passing
any arguments:

`ign gazebo`

You should see an empty world with several plugins loaded by default, such as the
3D Scene, the play/pause button, etc.

@image html files/gui_config/default_gui.png

By default, you're loading this file:

`$HOME/.ignition/gazebo/gui.config`

That file is created the first time you load Ignition Gazebo. Once it is
created, Ignition will never write to it again unless you delete it. This
means that you can customize it with your preferences and they will be applied
every time Ignition is started!

Let's try customizing it:

1. Open this file with your favorite editor:

    `$HOME/.ignition/gazebo/gui.config`

2. Change `material_theme` from `Light` to `Dark`

3. Reload Gazebo:

    `ign gazebo`

Note how the UI is now in dark mode!

@image html files/gui_config/dark_gui.png

You'll often want to restore default settings or to use the latest default
provided by Ignition (when you update to a newer version for example). In
that case, just delete that file, and the next time Gazebo is started a new file
will be created with default values:

`rm $HOME/.ignition/gazebo/gui.config`

### SDF

Let's try overriding the default configuration from an SDF file. Open your
favorite editor and save this file as `fuel_preview.sdf`:

```
<?xml version="1.0" ?>
<sdf version="1.6">
  <world name="fuel_preview">
    <plugin
      filename="libignition-gazebo-scene-broadcaster-system.so"
      name="ignition::gazebo::systems::SceneBroadcaster">
    </plugin>

    <gui fullscreen="0">

      <!-- 3D scene -->
      <plugin filename="GzScene3D" name="3D View">
        <ignition-gui>
          <title>3D View</title>
          <property type="bool" key="showTitleBar">false</property>
          <property type="string" key="state">docked</property>
        </ignition-gui>

        <engine>ogre2</engine>
        <scene>scene</scene>
        <ambient_light>1.0 1.0 1.0</ambient_light>
        <background_color>0.4 0.6 1.0</background_color>
        <camera_pose>8.3 7 7.8 0 0.5 -2.4</camera_pose>
      </plugin>

    </gui>

    <include>
      <uri>https://fuel.ignitionrobotics.org/1.0/OpenRobotics/models/Sun</uri>
    </include>

    <include>
      <uri>https://fuel.ignitionrobotics.org/1.0/OpenRobotics/models/Gazebo</uri>
    </include>

  </world>
</sdf>
```

Now let's load this world:

`ign gazebo <path to>/fuel_preview.sdf`

Notice how the application has only one GUI plugin loaded, the 3D scene, as defined
on the SDF file above.

@image html files/gui_config/fuel_preview.png

If you delete the `<gui>` element from the file above and reload it, you'll see
the same model loaded into the default GUI layout.

@image html files/gui_config/fuel_preview_no_gui.png

### Command line

It's often inconvenient to embed your GUI layout directly into every SDF file.
But you also don't want to be editing the default config file every time you
want to start with a different layout. That's why Gazebo also supports loading
configuration files from the command line.

Let's start by creating a custom configuration file, but instead of editing by
hand, we'll create it from the UI.

1. Start loading the SDF world we created above, with the `<gui>` element back:

    `ign gazebo <path to>/fuel_preview.sdf`

2. Now from the top-right menu, choose to add the "View Angle" plugin. This
   plugin has convenient buttons to change the camera angle, try them out!

3. Undock the inserted plugin by clicking on the □  button on top of it.

4. Now that the plugin is floating, drag it to any place you want.

5. Finally, let's save our configuration by going to the top-left menu
   and choosing "Save client configuration as".

6. Save the file at a path of your choice, and name it `saved.config`.

7. Close Gazebo

    @image html files/gui_config/save_config.gif

8. Take a look at the saved file if you're curious, it will look a lot like
   the default file, but with more properties defined.

9. Finally, let's load the previous world, with our custom configuration:

    `ign gazebo <path to>/fuel_preview.sdf --gui-config <path to>saved.config`

10. Gazebo should open with your custom layout.

    @image html files/gui_config/cmd_line.png

**Tip**: From the top-left menu, you can choose "Save client configuration" to
save directly to `$HOME/.ignition/gazebo/gui.config`.

**Tip**: In case you want to use the default GUI layout without editing the SDF file to
remove `<gui>`, you can pass `--gui-config $HOME/.ignition/gazebo/gui.config`:

#### Options

The above gives you other plugins, but the 3D scene isn't nicely formatted to
view the model (blue background, diagonal camera angle...). In order to get all
the default plugins, but format the 3D scene as it is in the SDF, we can use the
`merge` option:

`ign gazebo --gui-config-option merge <path to>/fuel_preview.sdf`

@image html files/gui_config/fuel_preview_merge.png

