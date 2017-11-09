/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "maekawa.h"

bool_t
xdr_filename (XDR *xdrs, filename objp)
{
	register int32_t *buf;

	 if (!xdr_vector (xdrs, (char *)objp, MAXLEN,
		sizeof (char), (xdrproc_t) xdr_char))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_vtimestamp (XDR *xdrs, vtimestamp objp)
{
	register int32_t *buf;

	 if (!xdr_vector (xdrs, (char *)objp, 5,
		sizeof (int), (xdrproc_t) xdr_int))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_sender_id (XDR *xdrs, sender_id *objp)
{
	register int32_t *buf;

	 if (!xdr_int (xdrs, objp))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_recv_id (XDR *xdrs, recv_id *objp)
{
	register int32_t *buf;

	 if (!xdr_int (xdrs, objp))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_request (XDR *xdrs, request *objp)
{
	register int32_t *buf;

	 if (!xdr_filename (xdrs, objp->name))
		 return FALSE;
	 if (!xdr_int (xdrs, &objp->start))
		 return FALSE;
	 if (!xdr_vtimestamp (xdrs, objp->ts))
		 return FALSE;
	 if (!xdr_sender_id (xdrs, &objp->sid))
		 return FALSE;
	 if (!xdr_recv_id (xdrs, &objp->rid))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_filepart (XDR *xdrs, filepart objp)
{
	register int32_t *buf;

	 if (!xdr_opaque (xdrs, objp, MAXLEN))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_partreceive (XDR *xdrs, partreceive *objp)
{
	register int32_t *buf;

	 if (!xdr_filepart (xdrs, objp->data))
		 return FALSE;
	 if (!xdr_int (xdrs, &objp->bytes))
		 return FALSE;
	 if (!xdr_vtimestamp (xdrs, objp->ts))
		 return FALSE;
	 if (!xdr_sender_id (xdrs, &objp->sid))
		 return FALSE;
	 if (!xdr_recv_id (xdrs, &objp->rid))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_partsend (XDR *xdrs, partsend *objp)
{
	register int32_t *buf;

	 if (!xdr_filename (xdrs, objp->name))
		 return FALSE;
	 if (!xdr_filepart (xdrs, objp->data))
		 return FALSE;
	 if (!xdr_int (xdrs, &objp->bytes))
		 return FALSE;
	 if (!xdr_vtimestamp (xdrs, objp->ts))
		 return FALSE;
	 if (!xdr_sender_id (xdrs, &objp->sid))
		 return FALSE;
	 if (!xdr_recv_id (xdrs, &objp->rid))
		 return FALSE;
	 if (!xdr_int (xdrs, &objp->isStart))
		 return FALSE;
	 if (!xdr_int (xdrs, &objp->source))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_readfile_res (XDR *xdrs, readfile_res *objp)
{
	register int32_t *buf;

	 if (!xdr_int (xdrs, &objp->errno))
		 return FALSE;
	switch (objp->errno) {
	case 0:
		 if (!xdr_partreceive (xdrs, &objp->readfile_res_u.part))
			 return FALSE;
		break;
	default:
		break;
	}
	return TRUE;
}

bool_t
xdr_fileListing (XDR *xdrs, fileListing *objp)
{
	register int32_t *buf;

	int i;
	 if (!xdr_vector (xdrs, (char *)objp->list, MAXLEN,
		sizeof (filename), (xdrproc_t) xdr_filename))
		 return FALSE;
	 if (!xdr_vector (xdrs, (char *)objp->ts, MAXLEN,
		sizeof (vtimestamp), (xdrproc_t) xdr_vtimestamp))
		 return FALSE;
	 if (!xdr_int (xdrs, &objp->len))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_listfile_res (XDR *xdrs, listfile_res *objp)
{
	register int32_t *buf;

	 if (!xdr_int (xdrs, &objp->errno))
		 return FALSE;
	switch (objp->errno) {
	case 0:
		 if (!xdr_fileListing (xdrs, &objp->listfile_res_u.fileListing))
			 return FALSE;
		break;
	default:
		break;
	}
	return TRUE;
}