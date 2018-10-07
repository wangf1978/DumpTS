#ifndef _SAL_H_
#define _SAL_H_



/************************************************************************
*  Orcas SAL
************************************************************************/
#define _Deref_out_opt_


// Input parameters --------------------------

//   _In_ - Annotations for parameters where data is passed into the function, but not modified.
//          _In_ by itself can be used with non-pointer types (although it is redundant).

// e.g. void SetPoint( _In_ const POINT* pPT );
#define _In_
#define _In_opt_

// nullterminated 'in' parameters.
// e.g. void CopyStr( _In_z_ const char* szFrom, _Out_z_cap_(cchTo) char* szTo, size_t cchTo );
#define _In_z_
#define _In_opt_z_

// 'input' buffers with given size
#define _In_reads_(size)
#define _In_reads_opt_(size)
#define _In_reads_bytes_(size)
#define _In_reads_bytes_opt_(size)
#define _In_reads_opt_z_(size)
#define _In_reads_z_(size)


// Output parameters --------------------------

//   _Out_ - Annotations for pointer or reference parameters where data passed back to the caller.
//           These are mostly used where the pointer/reference is to a non-pointer type.
//           _Outptr_/_Outref) (see below) are typically used to return pointers via parameters.

// e.g. void GetPoint( _Out_ POINT* pPT );
#define _Out_
#define _Out_opt_

#define _Out_writes_(size)
#define _Out_writes_opt_(size)
#define _Out_writes_bytes_(size)
#define _Out_writes_bytes_opt_(size)
#define _Out_writes_z_(size)

#define _Out_writes_bytes_all_(size)
#define _Out_writes_bytes_all_opt_(size)


// Inout parameters ----------------------------

//   _Inout_ - Annotations for pointer or reference parameters where data is passed in and
//        potentially modified.
//          void ModifyPoint( _Inout_ POINT* pPT );
//          void ModifyPointByRef( _Inout_ POINT& pPT );

#define _Inout_
#define _Inout_opt_

// For modifying string buffers
//   void toupper( _Inout_z_ char* sz );
#define _Inout_z_
#define _Inout_opt_z_

// For modifying buffers with explicit element size
#define _Inout_updates_(size)


/************************************************************************
*  SAL 2 _Ouptr_ family of annotations
************************************************************************/
#define _Outptr_result_maybenull_
#define _Outptr_opt_result_maybenull_


#define __in_ecount(size)
#define __out_ecount(size) 
#define __inout_ecount(size)
#define __out_bcount(size)
#define __in_z 


#define __RPC__in_opt
#define __RPC__out
#define __RPC__deref_out


#define _Use_decl_annotations_


#define _Outptr_


#define _COM_Outptr_

#define _Check_return_

#define _COM_Outptr_result_maybenull_

#define _Deref_pre_maybenull_
#define _Deref_post_maybenull_

#define _Outptr_result_nullonfailure_

#define _Null_terminated_ 


#endif //_RPC_H_
