#include <linux/utsname.h>
#include <net/cfg80211.h>
#include "core.h"
#include "ethtool.h"

static void cfg80211_get_drvinfo(struct net_device *dev,
					struct ethtool_drvinfo *info)
{
	struct wireless_dev *wdev = dev->ieee80211_ptr;

	strlcpy(info->driver, wiphy_dev(wdev->wiphy)->driver->name,
		sizeof(info->driver));

	strlcpy(info->version, init_utsname()->release, sizeof(info->version));

	if (wdev->wiphy->fw_version[0])
		strncpy(info->fw_version, wdev->wiphy->fw_version,
			sizeof(info->fw_version));
	else
		strncpy(info->fw_version, "N/A", sizeof(info->fw_version));

	strlcpy(info->bus_info, dev_name(wiphy_dev(wdev->wiphy)),
		sizeof(info->bus_info));
}

static int cfg80211_get_regs_len(struct net_device *dev)
{
	/* For now, return 0... */
	return 0;
}

static void cfg80211_get_regs(struct net_device *dev, struct ethtool_regs *regs,
			void *data)
{
	struct wireless_dev *wdev = dev->ieee80211_ptr;

	regs->version = wdev->wiphy->hw_version;
	regs->len = 0;
}

static void cfg80211_get_ringparam(struct net_device *dev,
				   struct ethtool_ringparam *rp)
{
	struct wireless_dev *wdev = dev->ieee80211_ptr;
	struct cfg80211_registered_device *rdev = wiphy_to_dev(wdev->wiphy);

	memset(rp, 0, sizeof(*rp));

	if (rdev->ops->get_ringparam)
		rdev->ops->get_ringparam(wdev->wiphy,
					 &rp->tx_pending, &rp->tx_max_pending,
					 &rp->rx_pending, &rp->rx_max_pending);
}

static int cfg80211_set_ringparam(struct net_device *dev,
				  struct ethtool_ringparam *rp)
{
	struct wireless_dev *wdev = dev->ieee80211_ptr;
	struct cfg80211_registered_device *rdev = wiphy_to_dev(wdev->wiphy);

	if (rp->rx_mini_pending != 0 || rp->rx_jumbo_pending != 0)
		return -EINVAL;

	if (rdev->ops->set_ringparam)
		return rdev->ops->set_ringparam(wdev->wiphy,
						rp->tx_pending, rp->rx_pending);

	return -ENOTSUPP;
}

const struct ethtool_ops cfg80211_ethtool_ops = {
	//.get_drvinfo = cfg80211_get_drvinfo,  /* work around solution */
	.get_regs_len = cfg80211_get_regs_len,
	.get_regs = cfg80211_get_regs,
	.get_link = ethtool_op_get_link,
	.get_ringparam = cfg80211_get_ringparam,
	.set_ringparam = cfg80211_set_ringparam,
};
