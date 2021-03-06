/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright (c) 2002-2003, Network Appliance, Inc. All rights reserved.
 */

/*
 * Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/*
 *
 * MODULE: dapl_evd_connection_callback.c
 *
 * PURPOSE: implements connection callbacks
 *
 * Description: Accepts asynchronous callbacks from the Communications Manager
 *              for EVDs that have been specified as the connection_evd.
 *
 * $Id: dapl_evd_connection_callb.c,v 1.33 2003/07/30 18:13:38 hobie16 Exp $
 */

#include "dapl.h"
#include "dapl_evd_util.h"
#include "dapl_ep_util.h"


/*
 * dapl_evd_connection_callback
 *
 * Connection callback function for ACTIVE connection requests; callbacks
 * generated by the Connection Manager in response to issuing a
 * connect call.
 *
 * Input:
 * 	ib_cm_handle,
 * 	ib_cm_event
 *	private_data_ptr
 * 	context (evd)
 *	cr_pp
 *
 * Output:
 * 	None
 *
 */

void
dapl_evd_connection_callback(
    IN    ib_cm_handle_t	ib_cm_handle,
    IN    const ib_cm_events_t  ib_cm_event,
    IN	  const void 		*private_data_ptr,
    IN    const void		*context)
{
	DAPL_EP		*ep_ptr;
	DAPL_EVD	*evd_ptr;
	DAPL_PRIVATE	*prd_ptr;
	DAT_EVENT_NUMBER event_type;

	dapl_dbg_log(
	    DAPL_DBG_TYPE_CM | DAPL_DBG_TYPE_CALLBACK,
	    "--> dapl_evd_connection_callback: ctxt: %p event: %x"
	    " cm_handle %p\n",
	    context,
	    ib_cm_event,
	    ib_cm_handle);

	/*
	 * Determine the type of handle passed back to us in the context
	 * and sort out key parameters.
	 */
	dapl_os_assert(((DAPL_HEADER *)context)->magic == DAPL_MAGIC_EP ||
	    ((DAPL_HEADER *)context)->magic == DAPL_MAGIC_EP_EXIT);
	/*
	 * Active side of the connection, context is an EP and
	 * PSP is irrelevant.
	 */
	ep_ptr  = (DAPL_EP *)context;
	evd_ptr = (DAPL_EVD *)ep_ptr->param.connect_evd_handle;

	prd_ptr = (DAPL_PRIVATE *)private_data_ptr;

	switch (ib_cm_event) {
	case IB_CME_CONNECTED:
	{
		/*
		 * If we don't have an EP at this point we are very screwed
		 * up
		 */
		DAT_RETURN dat_status;

		if (ep_ptr->param.ep_state == DAT_EP_STATE_DISCONNECT_PENDING) {
			/*
			 * If someone pulled the plug on the connection, just
			 * exit
			 */
			break;
		}
		dapls_ib_connected(ep_ptr);
		ep_ptr->param.ep_state	= DAT_EP_STATE_CONNECTED;
		ep_ptr->cm_handle	= ib_cm_handle;
		/* copy in the private data */
		(void) dapl_os_memcpy(ep_ptr->private_data,
		    prd_ptr->private_data,
		    IB_MAX_REQ_PDATA_SIZE);

		dat_status = dapls_evd_post_connection_event(
		    evd_ptr,
		    DAT_CONNECTION_EVENT_ESTABLISHED,
		    (DAT_HANDLE) ep_ptr,
		    IB_MAX_REQ_PDATA_SIZE,
		    ep_ptr->private_data);

		if (dat_status != DAT_SUCCESS) {
			(void) dapls_ib_disconnect(ep_ptr,
			    DAT_CLOSE_ABRUPT_FLAG);
			ep_ptr->param.ep_state =
			    DAT_EP_STATE_DISCONNECT_PENDING;
		}

		/*
		 * If we received any premature DTO completions and
		 * post them to the recv evd now.
		 * there is a race here - if events arrive after we change
		 * the ep state to connected and before we process premature
		 * events
		 */
		dapls_evd_post_premature_events(ep_ptr);

		break;
	}
	case IB_CME_DISCONNECTED:
	case IB_CME_DISCONNECTED_ON_LINK_DOWN:
	{
		/*
		 * EP is now fully disconnected; initiate any post processing
		 * to reset the underlying QP and get the EP ready for
		 * another connection
		 */
		if (ep_ptr->param.ep_state  == DAT_EP_STATE_DISCONNECTED) {
			/* DTO error caused this */
			event_type = DAT_CONNECTION_EVENT_BROKEN;
		} else {
			ep_ptr->param.ep_state  = DAT_EP_STATE_DISCONNECTED;
			dapls_ib_disconnect_clean(ep_ptr, DAT_TRUE,
			    ib_cm_event);
			event_type = DAT_CONNECTION_EVENT_DISCONNECTED;
		}

		/* If the EP has been freed, the evd_ptr will be NULL */
		if (evd_ptr != NULL) {
			(void) dapls_evd_post_connection_event(
			    evd_ptr, event_type, (DAT_HANDLE) ep_ptr, 0, 0);
		}

		/*
		 * If the user has done an ep_free of the EP, we have been
		 * waiting for the disconnect event; just clean it up now.
		 */
		if (ep_ptr->header.magic == DAPL_MAGIC_EP_EXIT) {
			(void) dapl_ep_free(ep_ptr);
		}
		break;
	}
	case IB_CME_DESTINATION_REJECT_PRIVATE_DATA:
	{
		ep_ptr->param.ep_state  = DAT_EP_STATE_DISCONNECTED;
		dapls_ib_disconnect_clean(ep_ptr, DAT_TRUE, ib_cm_event);
		(void) dapls_evd_post_connection_event(
		    evd_ptr,
		    DAT_CONNECTION_EVENT_PEER_REJECTED,
		    (DAT_HANDLE) ep_ptr,
		    0,
		    0);
		break;
	}
	case IB_CME_DESTINATION_UNREACHABLE:
	{
		ep_ptr->param.ep_state  = DAT_EP_STATE_DISCONNECTED;
		dapls_ib_disconnect_clean(ep_ptr, DAT_TRUE, ib_cm_event);
		(void) dapls_evd_post_connection_event(
		    evd_ptr,
		    DAT_CONNECTION_EVENT_UNREACHABLE,
		    (DAT_HANDLE) ep_ptr,
		    0,
		    0);
		break;
	}
	case IB_CME_DESTINATION_REJECT:
	case IB_CME_TOO_MANY_CONNECTION_REQUESTS:
	case IB_CME_LOCAL_FAILURE:
	{
		ep_ptr->param.ep_state  = DAT_EP_STATE_DISCONNECTED;
		dapls_ib_disconnect_clean(ep_ptr, DAT_TRUE, ib_cm_event);
		(void) dapls_evd_post_connection_event(
		    evd_ptr,
		    DAT_CONNECTION_EVENT_NON_PEER_REJECTED,
		    (DAT_HANDLE) ep_ptr,
		    0,
		    0);
		break;
	}
	case IB_CME_TIMED_OUT:
	{
		ep_ptr->param.ep_state  = DAT_EP_STATE_DISCONNECTED;
		dapls_ib_disconnect_clean(ep_ptr, DAT_TRUE, ib_cm_event);
		(void) dapls_evd_post_connection_event(
		    evd_ptr,
		    DAT_CONNECTION_EVENT_TIMED_OUT,
		    (DAT_HANDLE) ep_ptr,
		    0,
		    0);
		break;
	}
	case IB_CME_CONNECTION_REQUEST_PENDING:
	case IB_CME_CONNECTION_REQUEST_PENDING_PRIVATE_DATA:
	default:
	{
		dapl_os_assert(0);		/* shouldn't happen */
		break;
	}
	}

	dapl_dbg_log(DAPL_DBG_TYPE_CM | DAPL_DBG_TYPE_CALLBACK,
	    "dapl_evd_connection_callback () returns\n");

}


/*
 * Local variables:
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */
