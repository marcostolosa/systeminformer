/*
 * Copyright (c) 2022 Winsider Seminars & Solutions, Inc.  All rights reserved.
 *
 * This file is part of System Informer.
 *
 * Authors:
 *
 *     jxy-s   2022
 *
 */

#include <kphmsgdyn.h>

#include <pshpack1.h>
typedef struct _KPH_DYN_DATA_BUFFER
{
    USHORT Size;
    CHAR Buffer[ANYSIZE_ARRAY];

} KPH_DYN_DATA_BUFFER, *PKPH_DYN_DATA_BUFFER;
#include <poppack.h>

/**
 * \brief Finds a dynamic data entry in the table.
 * 
 * \param Message Message to get the table entry from.
 * \param FieldId Field identifier to look for.
 * 
 * \return Pointer to table entry on success, null if not found.
 */
_Must_inspect_result_
PCKPH_MESSAGE_DYNAMIC_TABLE_ENTRY KphpMsgDynFindEntry(
    _In_ PCKPH_MESSAGE Message,
    _In_ KPH_MESSAGE_FIELD_ID FieldId
    )
{
    for (USHORT i = 0; i < Message->_Dyn.Count; i++)
    {
        if (Message->_Dyn.Entries[i].FieldId == FieldId)
        {
            return &Message->_Dyn.Entries[i];
        }
    }

    return NULL;
}

/**
 * \brief Claims some data in the dynamic data buffer.
 * 
 * \param Message Message to claim dynamic data in.
 * \param FieldId Field identifier of the data to be populated.
 * \param TypeId Type identifier of the data to be populated.
 * \param RequiredSize Required size of the dynamic data.
 * \param DynData Set to pointer to claimed data in buffer on success.
 * 
 * \return Successful or errant status.
 */
_Must_inspect_result_
NTSTATUS KphpMsgDynClaimDynData(
    _In_ PKPH_MESSAGE Message,
    _In_ KPH_MESSAGE_FIELD_ID FieldId,
    _In_ KPH_MESSAGE_TYPE_ID TypeId,
    _In_ ULONG RequiredSize,
    _Outptr_result_nullonfailure_ PVOID* DynData
    )
{
    NTSTATUS status;
    PKPH_MESSAGE_DYNAMIC_TABLE_ENTRY claimed;
    ULONG offset;

    *DynData = NULL;

    if ((FieldId <= InvalidKphMsgField) || (FieldId >= MaxKphMsgField))
    {
        return STATUS_INVALID_PARAMETER_2;
    }

    if ((TypeId <= InvalidKphMsgType) || (TypeId >= MaxKphMsgType))
    {
        return STATUS_INVALID_PARAMETER_3;
    }

    status = KphMsgValidate(Message);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    if ((Message->_Dyn.Count >= ARRAYSIZE(Message->_Dyn.Entries)) ||
        (RequiredSize >= ARRAYSIZE(Message->_Dyn.Buffer)))
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (KphpMsgDynFindEntry(Message, FieldId) != NULL)
    {
        return STATUS_ALREADY_COMMITTED;
    }

    if (Message->_Dyn.Count == 0)
    {
        offset = 0;
        claimed = &Message->_Dyn.Entries[0];
    }
    else
    {
        ULONG endOffset;

        status = RtlULongAdd(
            Message->_Dyn.Entries[Message->_Dyn.Count - 1].Offset,
            Message->_Dyn.Entries[Message->_Dyn.Count - 1].Size,
            &offset);
        if (!NT_SUCCESS(status))
        {
            return status;
        }

        status = RtlULongAdd(offset, RequiredSize, &endOffset);
        if (!NT_SUCCESS(status))
        {
            return status;
        }

        if (endOffset >= ARRAYSIZE(Message->_Dyn.Buffer))
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        claimed = &Message->_Dyn.Entries[Message->_Dyn.Count];
    }

    claimed->FieldId = FieldId;
    claimed->TypeId = TypeId;
    claimed->Offset = offset;
    claimed->Size = RequiredSize;
    Message->_Dyn.Count++;
    Message->Header.Size += RequiredSize;
    *DynData = &Message->_Dyn.Buffer[offset];
    return STATUS_SUCCESS;
}

/**
 * \brief Looks up a dynamic data entry.
 * 
 * \param Message Message to look up dynamic data in.
 * \param FieldId Field identifier to look up.
 * \param TypeId Type identifier to look up.
 * \param DynData Set to point to dynamic data in buffer on success.
 *
 * \return Successful or errant status.
 */
_Must_inspect_result_
NTSTATUS KphpMsgDynLookupDynData(
    _In_ PCKPH_MESSAGE Message,
    _In_ KPH_MESSAGE_FIELD_ID FieldId,
    _In_ KPH_MESSAGE_TYPE_ID TypeId,
    _Outptr_result_nullonfailure_ const VOID** DynData
    )
{
    NTSTATUS status;
    PCKPH_MESSAGE_DYNAMIC_TABLE_ENTRY entry;
    ULONG offset;

    *DynData = NULL;

    status = KphMsgValidate(Message);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    entry = KphpMsgDynFindEntry(Message, FieldId);
    if (!entry)
    {
        return STATUS_NOT_FOUND;
    }

    if (entry->TypeId != TypeId)
    {
        return STATUS_CONTEXT_MISMATCH;
    }

    status = RtlULongAdd(entry->Offset, entry->Size, &offset);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    if (offset >= ARRAYSIZE(Message->_Dyn.Buffer))
    {
        return STATUS_HEAP_CORRUPTION;
    }

    *DynData = &Message->_Dyn.Buffer[entry->Offset];
    return STATUS_SUCCESS;
}

/**
 * \brief Clears the dynamic data table, effectively "freeing" the dynamic data
 * buffer to be populated with other information.
 *
 * \param Message Message to clear the dynamic data table of.
 */
VOID KphMsgDynClear(
    _Inout_ PKPH_MESSAGE Message
    )
{
    Message->Header.Size = KPH_MESSAGE_MIN_SIZE;
    Message->_Dyn.Count = 0;
    RtlZeroMemory(&Message->_Dyn.Entries, sizeof(Message->_Dyn.Entries));
}

/**
 * \brief Adds a unicode string to the dynamic data.
 * 
 * \param Message Message to populate dynamic data of.
 * \param FieldId Field identifier for the data.
 * \param String Unicode string to copy into the dynamic data.
 *
 * \return Successful or errant status.
 */
_Must_inspect_result_
NTSTATUS KphMsgDynAddUnicodeString(
    _Inout_ PKPH_MESSAGE Message,
    _In_ KPH_MESSAGE_FIELD_ID FieldId,
    _In_ PCUNICODE_STRING String 
    )
{
    NTSTATUS status;
    PKPH_DYN_DATA_BUFFER data;
    ULONG requiredSize;

    requiredSize = sizeof(KPH_DYN_DATA_BUFFER);
    status = RtlULongAdd(requiredSize, String->Length, &requiredSize);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    status = RtlULongAdd(requiredSize, sizeof(WCHAR), &requiredSize);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    status = KphpMsgDynClaimDynData(Message,
                                    FieldId,
                                    KphMsgTypeUnicodeString,
                                    requiredSize,
                                    (PVOID*)&data);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    data->Size = String->Length;
    RtlCopyMemory(&data->Buffer[0], String->Buffer, String->Length);
    *((PWCHAR)&data->Buffer[String->Length]) = L'\0';
    return STATUS_SUCCESS;
}

/**
 * \brief Retrieves a unicode string from the dynamic data of a message.
 *
 * \param Message Message to retrieve the unicode string from.
 * \param FieldId Field identifier for the data.
 * \param String If found populated with a reference to the string data in the
 * dynamic data buffer.
 *
 * \return Successful or errant status.
 */
_Must_inspect_result_
NTSTATUS KphMsgDynGetUnicodeString(
    _In_ PCKPH_MESSAGE Message,
    _In_ KPH_MESSAGE_FIELD_ID FieldId,
    _Out_ PUNICODE_STRING String 
    )
{
    NTSTATUS status;
    PKPH_DYN_DATA_BUFFER data;

    status = KphpMsgDynLookupDynData(Message,
                                     FieldId,
                                     KphMsgTypeUnicodeString,
                                     (PVOID*)&data);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    String->Length = data->Size;
    String->MaximumLength = data->Size;
    String->Buffer = (PWCH)&data->Buffer[0];
    return STATUS_SUCCESS;
}

/**
 * \brief Adds an ANSI string to the dynamic data buffer.
 * 
 * \param Message Message to add the string to.
 * \param FieldId Field identifier for the string.
 * \param String ANSI string to copy into the dynamic data buffer.
 * 
 * \return Successful or errant status. 
 */
_Must_inspect_result_
NTSTATUS KphMsgDynAddAnsiString(
    _Inout_ PKPH_MESSAGE Message,
    _In_ KPH_MESSAGE_FIELD_ID FieldId,
    _In_ PCANSI_STRING String 
    )
{
    NTSTATUS status;
    PKPH_DYN_DATA_BUFFER data;
    ULONG requiredSize;

    requiredSize = sizeof(KPH_DYN_DATA_BUFFER);
    status = RtlULongAdd(requiredSize, String->Length, &requiredSize);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    status = RtlULongAdd(requiredSize, sizeof(CHAR), &requiredSize);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    status = KphpMsgDynClaimDynData(Message,
                                    FieldId,
                                    KphMsgTypeAnsiString,
                                    requiredSize,
                                    (PVOID*)&data);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    data->Size = String->Length;
    RtlCopyMemory(&data->Buffer[0], String->Buffer, String->Length);
    data->Buffer[String->Length] = '\0';
    return STATUS_SUCCESS;
}

/**
 * \brief Retrieves an ANSI string from the message.
 *
 * \param Message Message to retrieve the string from.
 * \param FieldId Field identifier of the string.
 * \param String If found populated with a reference to the string data in the
 * dynamic data buffer.
 * 
 * \return Successful or errant status. 
 */
_Must_inspect_result_
NTSTATUS KphMsgDynGetAnsiString(
    _In_ PCKPH_MESSAGE Message,
    _In_ KPH_MESSAGE_FIELD_ID FieldId,
    _Out_ PANSI_STRING String 
    )
{
    NTSTATUS status;
    PKPH_DYN_DATA_BUFFER data;

    status = KphpMsgDynLookupDynData(Message,
                                     FieldId,
                                     KphMsgTypeAnsiString,
                                     (PVOID*)&data);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    String->Length = data->Size;
    String->MaximumLength = data->Size;
    String->Buffer = (PCHAR)&data->Buffer[0];
    return STATUS_SUCCESS;
}
