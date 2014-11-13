Todo
====

1.  Fix display order. (BUG 1)
    When creating a trigger, must update the display order of all
    following triggers.
2.  Allow to save scenario without first selecting ‘New’ from menu.

Bugs
----

1.  Display order not appearing on new triggers.
    The ‘duplicate trigger for all players’ button creates triggers with
    the same display order (which shouldn't happen).
    ### Symptoms ###
    Sync ID to DO button consequently moves triggers to the wrong
    location.

2.  Can't save scenario if haven't yet selected ‘new’ from the menu.
    This means if we create some triggers unwittingly then try to save,
    we can't.