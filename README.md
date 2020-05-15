# Resave Nodes

The Resave Nodes module allows for the automatic resaving of nodes.  The
resaving of nodes can be handled automatically through Backdrop's cron
functionality. The nodes that are resaved can be either the ones that have been
created or changed since the last time the module ran (via cron) or it can
be all nodes regardless of created/change date.  It can also be restricted to
nodes of selected content types.

Why do this?

In certain situations, importing nodes with geolocation information doesn't
always cause the geolookup to occur.  Resaving the node by hand triggers
that geolookup.  This by itself is fine if there aren't that many nodes and
the importing occurs manually.  But when hundreds (or more) nodes are
imported automatically on a recurring schedule, manually resaving
individual nodes or using the Views Bulk Operations module to manually bulk
save nodes is not a realistic option.  Enter this module.

Upon installation, configure the desired content types to monitor and
whether or not to do only those nodes that have been recently
created/changed. Then, select the cron option and the next time cron is run, 
nodes of that content type will be resaved, forcing the computing of that 
special field. You can also use the "Resave Nodes Now" button to do it
immediately.

## Installation

- Install this module using the [official Backdrop CMS instructions](https://backdropcms.org/guide/modules).

## Configuration and Usage

There are three main configuration options accessible under
'Configuration > System > Resave Nodes' or by going to
/admin/config/system/resave_nodes:

- Content type(s) to limit which nodes are resaved.
- Whether to go with the default "resave only created/changed nodes" or to
  resave all nodes regardless of created/changed time.
- Schedule the resaving of nodes via Backdrop's cron utility.

More details may be found (or added) in the [Wiki](https://github.com/backdrop-contrib/resave_nodes/wiki)

## Permissions

If someone other than user 1 needs to configure which content types to
monitor, set the "Administer the periodic resaving of nodes via cron"
permission for that user's role.

## Issues

Bugs and Feature requests should be reported in the [Issue Queue](https://github.com/backdrop-contrib/jitsi/issues)

## Current Maintainers

- [Laryn Kragt Bakker](https://github.com/laryn), [CEDC.org](https://CEDC.org)
- Collaboration and co-maintainers welcome!

## Credits

- Created for Drupal by [Adam Fuller (dasfuller)](https://drupal.org/user/2731951)

## License

This project is GPL-2.0 (or later) software. See the LICENSE.txt file in this
directory for complete text.
