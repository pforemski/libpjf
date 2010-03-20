/*
 * linux - various utilities specific for Linux
 *
 * This file is part of libasn
 * Copyright (C) 2005-2009 ASN Sp. z o.o.
 * Author: Pawel Foremski <pjf@asn.pl>
 *
 * libasn is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * libasn is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <error.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "lib.h"

ut *asn_ipa(bool index_by_ip, mmatic *mm)
{
	int len;
	char buf[16384];

	/*** prepare a RTNETLINK request ***/
	struct { struct nlmsghdr n; struct ifaddrmsg r; } req;
	memset(&req, 0, sizeof(req));

	req.n.nlmsg_len   = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
	req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ROOT;  /* NLM_F_ROOT = fetch all */
	req.n.nlmsg_type  = RTM_GETADDR;
	req.r.ifa_family  = AF_INET;                     /* fetch IPv4 */

	/*** send ***/
	int fd;

	fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
	if (fd < 0) reterrno(NULL, 0, "socket");

	len = send(fd, &req, req.n.nlmsg_len, 0);
	if (len < 0) reterrno(NULL, 0, "send");

	/* XXX: tested on a 2.4 host with 59 interfaces and one recv() seems enough */
	len = recv(fd, buf, sizeof(buf), 0);
	if (len < 0) reterrno(NULL, 1, "recv");

	/*** parse ***/
	struct nlmsghdr *h1;
	struct ifaddrmsg *h2;
	struct rtattr *h3;
	int h3len;

	char *interface;
	char address[INET_ADDRSTRLEN];

	ut *db = ut_new_thash(NULL, mm);
	thash *dbh = ut_thash(db);
	ut *list;

	for (h1 = (struct nlmsghdr *) buf;
	     h1->nlmsg_type != NLMSG_DONE && NLMSG_OK(h1, len);
	     h1 = NLMSG_NEXT(h1, len)) {

		h2 = (struct ifaddrmsg *) NLMSG_DATA(h1);
		dbg(10, "parsing iface #%d\n", h2->ifa_index);

		for (h3 = IFA_RTA(h2), h3len = IFA_PAYLOAD(h1);
		     RTA_OK(h3, h3len);
		     h3 = RTA_NEXT(h3, h3len)) {

			switch (h3->rta_type) {
				case IFA_ADDRESS:
					inet_ntop(AF_INET, RTA_DATA(h3), address, sizeof(address));
					break;
				case IFA_LABEL:
					interface = RTA_DATA(h3);
					break;
			}
		}

		/*** write the answer ***/
		if (index_by_ip) {
			uth_add_char(db, address, interface);
		} else {
			if (!(list = thash_get(dbh, interface)))
				list = uth_add_tlist(db, interface, NULL);

			utl_add_char(list, address);
		}
	}

	return db;
}

#include "lib.h"

