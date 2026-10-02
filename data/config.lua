-- TDE desktop configuration, shared by all TDE applications.
--
-- TDE applications create ~/.config/tde/config.lua from this when it is missing; change
-- what you like, and open applications follow as soon as it is saved. Every setting is
-- optional: anything left out keeps the default shown here. Application specific settings
-- live next to it, in ~/.config/tde/<application>/config.lua.
--
-- The original is in /usr/share/doc/libtde/examples.

return {
    window_buttons = {
        -- Which end of the header bar holds the window buttons: "left" or "right".
        position = "right",
        -- The buttons to show, in order: any of "minimize", "maximize" and "close".
        order = { "minimize", "maximize", "close" },
    },

    -- Terminal to open ("Open in Terminal", terminal applications). By default $TERMINAL,
    -- else the first one installed of the usual ones.
    -- terminal = "kitty",

    appearance = {
        theme = "arc-dark",     -- "arc-dark", "arc" or "system" (follows the light/dark preference)

        -- Roundness of buttons, entries, selections and popups, in pixels. 0 makes them square.
        corner_radius = 5,      -- 0 to 24

        -- Icon theme; by default the one the rest of the desktop uses.
        -- icon_theme = "Papirus-Dark",

        -- Override single theme colours. Names: window, base, header, sidebar, sidebar_text,
        -- text, dim_text, accent, accent_text, border, hover, pressed, entry, scrollbar,
        -- close_hover, error. Values are "#rrggbb" or "#aarrggbb".
        -- colors = {
        --     accent = "#5294e2",
        -- },
    },

    lock = {
        -- Minutes without input before the screen locks; 0 locks only when asked, and
        -- before sleeping. Programs such as video players keep it from locking meanwhile.
        after = 5,              -- 0 to 1440
    },
}
