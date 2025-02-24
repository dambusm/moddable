/*
 * Copyright (c) 2016-2017  Moddable Tech, Inc.
 *
 *   This file is part of the Moddable SDK Runtime.
 * 
 *   The Moddable SDK Runtime is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 * 
 *   The Moddable SDK Runtime is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with the Moddable SDK Runtime.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This file incorporates work covered by the following copyright and  
 * permission notice:  
 *
 *       Copyright (C) 2010-2016 Marvell International Ltd.
 *       Copyright (C) 2002-2010 Kinoma, Inc.
 *
 *       Licensed under the Apache License, Version 2.0 (the "License");
 *       you may not use this file except in compliance with the License.
 *       You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *       Unless required by applicable law or agreed to in writing, software
 *       distributed under the License is distributed on an "AS IS" BASIS,
 *       WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *       See the License for the specific language governing permissions and
 *       limitations under the License.
 */

#include "xsAll.h"
#if mxWindows
	#include <Winnls.h>
#elif mxMacOSX
	#include <CoreServices/CoreServices.h>
#elif mxiOS
	#include <CoreFoundation/CoreFoundation.h>
#endif

#define mxStringInstanceLength(INSTANCE) ((txIndex)fxUnicodeLength(instance->next->value.string))

static void fx_String_prototype_replaceAux(txMachine* the, txInteger size, txInteger offset, txSlot* function, txSlot* match, txInteger matchLength, txSlot* replace);
static txSlot* fx_String_prototype_split_aux(txMachine* the, txSlot* theString, txSlot* theArray, txSlot* theItem, txInteger theStart, txInteger theStop);

static txSlot* fxCheckString(txMachine* the, txSlot* it);
static txString fxCoerceToString(txMachine* the, txSlot* theSlot);
static txInteger fxArgToPosition(txMachine* the, txInteger i, txInteger index, txInteger length);
static void fx_String_prototype_pad(txMachine* the, txBoolean flag);
static void fx_String_prototype_trimAux(txMachine* the, txBoolean trimStart, txBoolean trimEnd);
static txBoolean fx_String_prototype_withRegexp(txMachine* the, txID id, txBoolean global, txInteger count);
static void fx_String_prototype_withoutRegexp(txMachine* the, txID id, txBoolean global, txInteger count);

static txBoolean fxStringDeleteProperty(txMachine* the, txSlot* instance, txID id, txIndex index);
static txBoolean fxStringDefineOwnProperty(txMachine* the, txSlot* instance, txID id, txIndex index, txSlot* slot, txFlag mask);
static txBoolean fxStringGetOwnProperty(txMachine* the, txSlot* instance, txID id, txIndex index, txSlot* slot);
static txSlot* fxStringGetProperty(txMachine* the, txSlot* instance, txID id, txIndex index, txFlag flag);
static txBoolean fxStringHasProperty(txMachine* the, txSlot* instance, txID id, txIndex index);
static void fxStringOwnKeys(txMachine* the, txSlot* instance, txFlag flag, txSlot* keys);
static txSlot* fxStringSetProperty(txMachine* the, txSlot* instance, txID id, txIndex index, txFlag flag);

const txBehavior ICACHE_FLASH_ATTR gxStringBehavior = {
	fxStringGetProperty,
	fxStringSetProperty,
	fxOrdinaryCall,
	fxOrdinaryConstruct,
	fxStringDefineOwnProperty,
	fxStringDeleteProperty,
	fxStringGetOwnProperty,
	fxOrdinaryGetPropertyValue,
	fxOrdinaryGetPrototype,
	fxStringHasProperty,
	fxOrdinaryIsExtensible,
	fxStringOwnKeys,
	fxOrdinaryPreventExtensions,
	fxOrdinarySetPropertyValue,
	fxOrdinarySetPrototype,
};

void fxBuildString(txMachine* the)
{
	txSlot* slot;
	
	fxNewHostFunction(the, mxCallback(fxStringAccessorGetter), 0, XS_NO_ID);
	fxNewHostFunction(the, mxCallback(fxStringAccessorSetter), 1, XS_NO_ID);
	mxPushUndefined();
	the->stack->flag = XS_DONT_DELETE_FLAG;
	the->stack->kind = XS_ACCESSOR_KIND;
	the->stack->value.accessor.getter = (the->stack + 2)->value.reference;
	the->stack->value.accessor.setter = (the->stack + 1)->value.reference;
	mxPull(mxStringAccessor);
	the->stack += 2;
	
	mxPush(mxObjectPrototype);
	slot = fxLastProperty(the, fxNewStringInstance(the));
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_at), 1, mxID(_at), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_charAt), 1, mxID(_charAt), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_charCodeAt), 1, mxID(_charCodeAt), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_codePointAt), 1, mxID(_codePointAt), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_compare), 1, mxID(_compare), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_concat), 1, mxID(_concat), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_endsWith), 1, mxID(_endsWith), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_includes), 1, mxID(_includes), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_indexOf), 1, mxID(_indexOf), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_lastIndexOf), 1, mxID(_lastIndexOf), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_localeCompare), 1, mxID(_localeCompare), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_normalize), 0, mxID(_normalize), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_padEnd), 1, mxID(_padEnd), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_padStart), 1, mxID(_padStart), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_repeat), 1, mxID(_repeat), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_replace), 2, mxID(_replace), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_replaceAll), 2, mxID(_replaceAll), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_slice), 2, mxID(_slice), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_split), 2, mxID(_split), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_startsWith), 1, mxID(_startsWith), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_substr), 2, mxID(_substr), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_substring), 2, mxID(_substring), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_toLowerCase), 0, mxID(_toLocaleLowerCase), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_toUpperCase), 0, mxID(_toLocaleUpperCase), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_toLowerCase), 0, mxID(_toLowerCase), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_valueOf), 0, mxID(_toString), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_toUpperCase), 0, mxID(_toUpperCase), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_trim), 0, mxID(_trim), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_trimEnd), 0, mxID(_trimEnd), XS_DONT_ENUM_FLAG);
	slot = fxNextSlotProperty(the, slot, slot, mxID(_trimRight), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_trimStart), 0, mxID(_trimStart), XS_DONT_ENUM_FLAG);
	slot = fxNextSlotProperty(the, slot, slot, mxID(_trimLeft), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_valueOf), 0, mxID(_valueOf), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_iterator), 0, mxID(_Symbol_iterator), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_match), 1, mxID(_match), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_matchAll), 1, mxID(_matchAll), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_search), 1, mxID(_search), XS_DONT_ENUM_FLAG);
	mxStringPrototype = *the->stack;
	slot = fxBuildHostConstructor(the, mxCallback(fx_String), 1, mxID(_String));
	mxStringConstructor = *the->stack;
	slot = fxLastProperty(the, slot);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_fromArrayBuffer), 1, mxID(_fromArrayBuffer), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_fromCharCode), 1, mxID(_fromCharCode), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_fromCodePoint), 1, mxID(_fromCodePoint), XS_DONT_ENUM_FLAG);
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_raw), 1, mxID(_raw), XS_DONT_ENUM_FLAG);
	mxPop();

	mxPush(mxIteratorPrototype);
	slot = fxLastProperty(the, fxNewObjectInstance(the));
	slot = fxNextHostFunctionProperty(the, slot, mxCallback(fx_String_prototype_iterator_next), 0, mxID(_next), XS_DONT_DELETE_FLAG | XS_DONT_ENUM_FLAG);
	slot = fxNextStringXProperty(the, slot, "String Iterator", mxID(_Symbol_toStringTag), XS_DONT_ENUM_FLAG | XS_DONT_SET_FLAG);
	mxPull(mxStringIteratorPrototype);
}

txSlot* fxNewStringInstance(txMachine* the)
{
	txSlot* instance;
	txSlot* property;
	instance = fxNewObjectInstance(the);
	instance->flag |= XS_EXOTIC_FLAG;
	property = fxNextSlotProperty(the, instance, &mxEmptyString, XS_STRING_BEHAVIOR, XS_INTERNAL_FLAG);
	return instance;
}

void fxStringAccessorGetter(txMachine* the)
{
	txSlot* string;
	txID id = the->scratch.value.at.id;
	txIndex index = the->scratch.value.at.index;
	if ((mxThis->kind == XS_STRING_KIND) || (mxThis->kind == XS_STRING_X_KIND))
		string = mxThis;
	else {
		txSlot* instance = fxToInstance(the, mxThis);
		while (instance) {
			if (instance->flag & XS_EXOTIC_FLAG) {
				string = instance->next;
				if (string->ID == XS_STRING_BEHAVIOR)
					break;
			}
			instance = fxGetPrototype(the, instance);
		}
	}
	if (id == mxID(_length)) {
		mxResult->value.integer = fxUnicodeLength(string->value.string);
		mxResult->kind = XS_INTEGER_KIND;
	}
	else {
		txInteger from = fxUnicodeToUTF8Offset(string->value.string, index);
		if (from >= 0) {
			txInteger to = from + fxUnicodeToUTF8Offset(string->value.string + from, 1);
			if (to >= 0) {
				mxResult->value.string = fxNewChunk(the, to - from + 1);
				c_memcpy(mxResult->value.string, string->value.string + from, to - from);
				mxResult->value.string[to - from] = 0;
				mxResult->kind = XS_STRING_KIND;
			}
		}
	}
}

void fxStringAccessorSetter(txMachine* the)
{
}

txBoolean fxStringDefineOwnProperty(txMachine* the, txSlot* instance, txID id, txIndex index, txSlot* slot, txFlag mask)
{
	if ((id == mxID(_length)) || (!id && (mxStringInstanceLength(instance) > index)))
		return 0;
	return fxOrdinaryDefineOwnProperty(the, instance, id, index, slot, mask);
}

txBoolean fxStringDeleteProperty(txMachine* the, txSlot* instance, txID id, txIndex index)
{
	if ((id == mxID(_length)) || (!id && (mxStringInstanceLength(instance) > index)))
		return 0;
	return fxOrdinaryDeleteProperty(the, instance, id, index);
}

txBoolean fxStringGetOwnProperty(txMachine* the, txSlot* instance, txID id, txIndex index, txSlot* descriptor)
{
	if (id == mxID(_length)) {
		descriptor->flag = XS_DONT_DELETE_FLAG | XS_DONT_ENUM_FLAG | XS_DONT_SET_FLAG;
		descriptor->ID = id;
		descriptor->kind = XS_INTEGER_KIND;
		descriptor->value.integer = mxStringInstanceLength(instance);
		return 1;
	}
	if (!id && (mxStringInstanceLength(instance) > index)) {
		txSlot* string = instance->next;
		txInteger from = fxUnicodeToUTF8Offset(string->value.key.string, index);
		txInteger to = fxUnicodeToUTF8Offset(string->value.key.string, index + 1);
		descriptor->value.string = fxNewChunk(the, to - from + 1);
		c_memcpy(descriptor->value.string, string->value.key.string + from, to - from);
		descriptor->value.string[to - from] = 0;
		descriptor->kind = XS_STRING_KIND;
		descriptor->flag = XS_DONT_DELETE_FLAG | XS_DONT_SET_FLAG;
		return 1;
	}
	return fxOrdinaryGetOwnProperty(the, instance, id, index, descriptor);
}

txSlot* fxStringGetProperty(txMachine* the, txSlot* instance, txID id, txIndex index, txFlag flag)
{
	if ((id == mxID(_length)) || (!id && (mxStringInstanceLength(instance) > index))) {
		the->scratch.value.at.id = id;
		the->scratch.value.at.index = index;
		return &mxStringAccessor;
	}
	return fxOrdinaryGetProperty(the, instance, id, index, flag);
}

txBoolean fxStringHasProperty(txMachine* the, txSlot* instance, txID id, txIndex index)
{
	if ((id == mxID(_length)) || (!id && (mxStringInstanceLength(instance) > index)))
		return 1;
	return fxOrdinaryHasProperty(the, instance, id, index);
}

void fxStringOwnKeys(txMachine* the, txSlot* instance, txFlag flag, txSlot* keys)
{
	txSlot* property = instance->next;
	if (flag & XS_EACH_NAME_FLAG) {
		txIndex length = mxStringInstanceLength(instance), index;
		for (index = 0; index < length; index++)
			keys = fxQueueKey(the, 0, index, keys);
	}
	property = property->next;
	if (property && (property->kind == XS_ARRAY_KIND)) {
		keys = fxQueueIndexKeys(the, property, flag, keys);
		property = property->next;
	}
	if (flag & XS_EACH_NAME_FLAG)
		keys = fxQueueKey(the, mxID(_length), 0, keys);
	fxQueueIDKeys(the, property, flag, keys);
}

txSlot* fxStringSetProperty(txMachine* the, txSlot* instance, txID id, txIndex index, txFlag flag)
{
	if ((id == mxID(_length)) || (!id && (mxStringInstanceLength(instance) > index)))
		return C_NULL;
	return fxOrdinarySetProperty(the, instance, id, index, flag);
}

void fx_String(txMachine* the)
{
	txSlot* slot;
	txSlot* instance;
	if (mxArgc > 0) {
		slot = mxArgv(0);
		if ((mxTarget->kind == XS_UNDEFINED_KIND) && (slot->kind == XS_SYMBOL_KIND)) {
			fxSymbolToString(the, slot);
			*mxResult = *slot;
			return;
		}
		fxToString(the, slot);
	}
	else {
		slot = &mxEmptyString;
	}
	if (mxTarget->kind == XS_UNDEFINED_KIND) {
		*mxResult = *slot;
		return;
	}
	mxPushSlot(mxTarget);
	fxGetPrototypeFromConstructor(the, &mxStringPrototype);
	instance = fxNewStringInstance(the);
	instance->next->kind = slot->kind; // @@
	instance->next->value.key.string = slot->value.string;
	mxPullSlot(mxResult);
}

void fx_String_fromArrayBuffer(txMachine* the)
{
	txSlot* slot;
	txSlot* arrayBuffer = C_NULL;
	txSlot* bufferInfo;
	txInteger limit, offset;
	txInteger inLength, outLength = 0;
	unsigned char *in;
	txString string;
	if (mxArgc < 1)
		mxTypeError("no argument");
	slot = mxArgv(0);
	if (slot->kind == XS_REFERENCE_KIND) {
		slot = slot->value.reference->next;
		if (slot && (slot->kind == XS_ARRAY_BUFFER_KIND))
			arrayBuffer = slot;
	}
	if (!arrayBuffer)
		mxTypeError("argument is no ArrayBuffer instance");
	bufferInfo = arrayBuffer->next;
	limit = bufferInfo->value.bufferInfo.length;
	offset = fxArgToByteLength(the, 1, 0);
	if (limit < offset)
		mxRangeError("out of range byteOffset %ld", offset);
	inLength = fxArgToByteLength(the, 2, limit - offset);
	if (limit < (offset + inLength))
		mxRangeError("out of range byteLength %ld", inLength);

	in = offset + (unsigned char *)arrayBuffer->value.arrayBuffer.address;
	while (inLength > 0) {
		unsigned char first = c_read8(in++), clen;
		if (first < 0x80){
			if (0 == first)
				break;
			inLength -= 1;
			outLength += 1;
			continue;
		}

		if (0xC0 == (first & 0xE0))
			clen = 2;
		else if (0xE0 == (first & 0xF0))
			clen = 3;
		else if (0xF0 == (first & 0xF0))
			clen = 4;
		else
			goto badUTF8;

		inLength -= clen;
		if (inLength < 0)
			goto badUTF8;

		outLength += clen;
		clen -= 1;
		do {
			if (0x80 != (0xc0 & c_read8(in++)))
				goto badUTF8;
		} while (--clen > 0);
	}

	string = fxNewChunk(the, outLength + 1);
	c_memcpy(string, offset + arrayBuffer->value.arrayBuffer.address, outLength);
	string[outLength] = 0;
	mxResult->value.string = string;
	mxResult->kind = XS_STRING_KIND;

	return;

badUTF8:
	mxTypeError("invalid UTF-8");
}

void fx_String_fromCharCode(txMachine* the)
{
	txInteger length = 0;
	txInteger count = mxArgc;
	txInteger index = 0;
	txInteger c, d; 
	txString p;
	while (index < count) {
		txNumber number = fxToNumber(the, mxArgv(index));
		switch (c_fpclassify(number)) {
		case C_FP_INFINITE:
		case C_FP_NAN:
		case C_FP_ZERO:
			mxArgv(index)->value.integer = 0;
			break;
		default:
			#define MODULO 65536.0
			number = c_fmod(c_trunc(number), MODULO);
			if (number < 0)
				number += MODULO;
			mxArgv(index)->value.integer = (txInteger)number;
			break;
		}
		mxArgv(index)->kind = XS_INTEGER_KIND;
		index++;
	}
	index = 0;
	while (index < count) {
		c = mxArgv(index)->value.integer;
		index++;
		if (index < count) {
			d = mxArgv(index)->value.integer;
			if ((0x0000D800 <= c) && (c <= 0x0000DBFF) && (0x0000DC00 <= d) && (d <= 0x0000DFFF)) {
				c = 0x00010000 + ((c & 0x000003FF) << 10) + (d & 0x000003FF);
				index++;
			}
		}	
		length += fxUTF8Length(c);
	}		
	mxMeterSome(count);
	mxResult->value.string = (txString)fxNewChunk(the, length + 1);
	mxResult->kind = XS_STRING_KIND;
	p = mxResult->value.string;
	index = 0;
	while (index < count) {
		c = mxArgv(index)->value.integer;
		index++;
		if (index < count) {
			d = mxArgv(index)->value.integer;
			if ((0x0000D800 <= c) && (c <= 0x0000DBFF) && (0x0000DC00 <= d) && (d <= 0x0000DFFF)) {
				c = 0x00010000 + ((c & 0x000003FF) << 10) + (d & 0x000003FF);
				index++;
			}
		}	
		p = fxUTF8Encode(p, c);
	}	
	*p = 0;
}

void fx_String_fromCodePoint(txMachine* the)
{
	txInteger length = 0;
	txInteger count = mxArgc;
	txInteger index = 0;
	txInteger c; 
	txString p;
	while (index < count) {
		txNumber number = fxToNumber(the, mxArgv(index));
		txNumber check = c_trunc(number);
		if (number != check)
			mxRangeError("invalid code point %lf", number);
		if ((number < 0) || (0x10FFFF < number))
			mxRangeError("invalid code point %lf", number);
		c = mxArgv(index)->value.integer = (txInteger)number;
		mxArgv(index)->kind = XS_INTEGER_KIND;
		length += fxUTF8Length(c);
		index++;
	}
	mxMeterSome(count);
	mxResult->value.string = (txString)fxNewChunk(the, length + 1);
	mxResult->kind = XS_STRING_KIND;
	p = mxResult->value.string;
	index = 0;
	while (index < count) {
		c = mxArgv(index)->value.integer;
		p = fxUTF8Encode(p, c);
		index++;
	}	
	*p = 0;
}

void fx_String_raw(txMachine* the)
{
	txInteger argCount = mxArgc;
	txSlot* raw;
	txInteger rawCount;
	if (argCount > 0)
		fxToInstance(the, mxArgv(0));
	else
		mxTypeError("cannot coerce undefined to object");
	mxPushSlot(mxArgv(0));
	mxGetID(mxID(_raw));
	raw = the->stack;
	mxPushSlot(raw);
	mxGetID(mxID(_length));
	rawCount = fxToInteger(the, the->stack);
	mxPop();
	if (rawCount <= 0) {
		mxResult->value = mxEmptyString.value;
	}
	else {
		txSlot* list;
		txInteger index = 0;
		txSlot* item;
		txInteger size;
		list = item = fxNewInstance(the);
		mxPushSlot(list);
		for (;;) {
			mxPushSlot(raw);
			mxGetIndex(index);
			fxToString(the, the->stack);
			item = item->next = fxNewSlot(the);
			mxPullSlot(item);
			index++;
			if (index == rawCount)
				break;
			if (index < argCount) {
				mxPushSlot(mxArgv(index));
				fxToString(the, the->stack);
			}
			else
				mxPush(mxEmptyString);
			item = item->next = fxNewSlot(the);
			mxPullSlot(item);
		}
		size = 0;
		item = list->next;
		while (item) {
			item->value.key.sum = mxStringLength(item->value.string);
			size += item->value.key.sum;
			item = item->next;
		}
		size++;
		mxResult->value.string = (txString)fxNewChunk(the, size);
		size = 0;
		item = list->next;
		while (item) {
			c_memcpy(mxResult->value.string + size, item->value.string, item->value.key.sum);
			size += item->value.key.sum;
			item = item->next;
		}
		mxResult->value.string[size] = 0;
		mxPop();
	}
	mxResult->kind = XS_STRING_KIND;
	mxPop();
}

void fx_String_prototype_at(txMachine* the)
{
	txString string = fxCoerceToString(the, mxThis);
	txNumber length = fxUnicodeLength(string);
	txNumber index = (mxArgc > 0) ? c_trunc(fxToNumber(the, mxArgv(0))) : C_NAN;
	if (c_isnan(index) || (index == 0))
		index = 0;
	if (index < 0)
		index = length + index;
	if ((0 <= index) && (index < length)) {
		txInteger from = fxUnicodeToUTF8Offset(string, (txIndex)index);
		if (from >= 0) {
			txInteger to = fxUnicodeToUTF8Offset(string, (txIndex)(index + 1));
			if (to >= 0) {
				mxResult->value.string = fxNewChunk(the, to - from + 1);
				c_memcpy(mxResult->value.string, fxToString(the, mxThis) + from, to - from);
				mxResult->value.string[to - from] = 0;
				mxResult->kind = XS_STRING_KIND;
			}
		}
	}
}

void fx_String_prototype_charAt(txMachine* the)
{
	txString aString;
	txInteger aLength;
	txInteger anOffset;

	aString = fxCoerceToString(the, mxThis);
	if ((mxArgc > 0) && (mxArgv(0)->kind != XS_UNDEFINED_KIND)) {
		anOffset = fxToInteger(the, mxArgv(0));
		if (anOffset < 0) goto fail;
	}
	else
		anOffset = 0;

	anOffset = fxUnicodeToUTF8Offset(aString, anOffset);
	if (anOffset < 0) goto fail;

	aLength = fxUnicodeToUTF8Offset(aString + anOffset, 1);
	if (aLength < 0) goto fail;

	mxResult->value.string = (txString)fxNewChunk(the, aLength + 1);
	c_memcpy(mxResult->value.string, mxThis->value.string + anOffset, aLength);
	mxResult->value.string[aLength] = 0;
	mxResult->kind = XS_STRING_KIND;
	return;

fail:
	mxResult->value.string = mxEmptyString.value.string;
	mxResult->kind = mxEmptyString.kind;
}

void fx_String_prototype_charCodeAt(txMachine* the)
{
	txString aString;
	txInteger anOffset;

	aString = fxCoerceToString(the, mxThis);
	if ((mxArgc > 0) && (mxArgv(0)->kind != XS_UNDEFINED_KIND)) {
		anOffset = fxToInteger(the, mxArgv(0));
		if (anOffset < 0) goto fail;
	}
	else
		anOffset = 0;

	anOffset = fxUnicodeToUTF8Offset(aString, anOffset);
	if (anOffset < 0) goto fail;

	if (fxUnicodeToUTF8Offset(aString + anOffset, 1) < 0)
		goto fail;

	fxUTF8Decode(aString + anOffset, &mxResult->value.integer);
	mxResult->kind = XS_INTEGER_KIND;
	return;

fail:
	mxResult->value.number = C_NAN;
	mxResult->kind = XS_NUMBER_KIND;
}

void fx_String_prototype_compare(txMachine* the)
{
	fxReport(the, "# Use standard String.prototype.localeCompare instead of soon obsolete String.prototype.compare\n");
	fx_String_prototype_localeCompare(the);
}

void fx_String_prototype_codePointAt(txMachine* the)
{
	txString string = fxCoerceToString(the, mxThis);
	txInteger length = fxUnicodeLength(string);
	txNumber at = (mxArgc > 0) ? fxToNumber(the, mxArgv(0)) : 0;
	if (c_isnan(at))
		at = 0;
	if ((0 <= at) && (at < (txNumber)length)) {
		txInteger offset = fxUnicodeToUTF8Offset(string, (txInteger)at);
		length = fxUnicodeToUTF8Offset(string + offset, 1);
		if ((offset >= 0) && (length > 0)) {
			fxUTF8Decode(string + offset, &mxResult->value.integer);
			mxResult->kind = XS_INTEGER_KIND;
		}
	}
}

void fx_String_prototype_concat(txMachine* the)
{
	txInteger aCount;
	txInteger aLength;
	txInteger anIndex;
	
	fxCoerceToString(the, mxThis);
	aCount = mxArgc;
	aLength = mxStringLength(mxThis->value.string);
	for (anIndex = 0; anIndex < aCount; anIndex++)
		aLength += mxStringLength(fxToString(the, mxArgv(anIndex)));
	mxResult->value.string = (txString)fxNewChunk(the, aLength + 1);
	mxResult->kind = XS_STRING_KIND;
	c_strcpy(mxResult->value.string, mxThis->value.string);
	for (anIndex = 0; anIndex < aCount; anIndex++)
		c_strcat(mxResult->value.string, mxArgv(anIndex)->value.string);
	mxMeterSome(aCount);
}

void fx_String_prototype_endsWith(txMachine* the)
{
	txString string = fxCoerceToString(the, mxThis);
	txInteger length = fxUnicodeLength(string);
	txString searchString;
	txInteger searchLength;
	txInteger offset;
	mxResult->kind = XS_BOOLEAN_KIND;
	mxResult->value.boolean = 0;
	if (mxArgc < 1)
		return;
	if (fxIsRegExp(the, mxArgv(0)))
		mxTypeError("future editions");
	searchString = fxToString(the, mxArgv(0));
	searchLength = mxStringLength(searchString);
	offset = fxUnicodeToUTF8Offset(string, fxArgToPosition(the, 1, length, length));
	if (offset < searchLength)
		return;
	mxMeterSome(fxUnicodeLength(searchString));
	if (!c_strncmp(string + offset - searchLength, searchString, searchLength))
		mxResult->value.boolean = 1;
}

static txString fx_String_prototype_includes_aux(txMachine* the, txString string, txSize stringLength, txString searchString, txSize searchLength)
{
	txString result = string;
	txString limit = string + stringLength - searchLength;
	while (result <= limit) {
		txU1 c;
		txU1* p = (txU1*)result;
		txU1* q = (txU1*)searchString;
		while ((c = c_read8(q)) && (c_read8(p) == c)) {
			mxMeterSome(((c & 0xC0) != 0x80) ? 1 : 0);
			p++;
			q++;
		}
		if (c)
			result++;
		else
			return result;
	}
	return C_NULL;
}

void fx_String_prototype_includes(txMachine* the)
{
	txString string = fxCoerceToString(the, mxThis);
	txInteger length = mxStringLength(string);
	txString searchString;
	txInteger searchLength;
	txInteger offset;
	mxResult->kind = XS_BOOLEAN_KIND;
	mxResult->value.boolean = 0;
	if (mxArgc < 1)
		return;
	if (fxIsRegExp(the, mxArgv(0)))
		mxTypeError("future editions");
	searchString = fxToString(the, mxArgv(0));
	searchLength = mxStringLength(searchString);
	offset = fxUnicodeToUTF8Offset(string, fxArgToPosition(the, 1, 0, fxUnicodeLength(string)));
	if ((length - offset) < searchLength)
		return;
	if (fx_String_prototype_includes_aux(the, string + offset, length - offset, searchString, searchLength))
		mxResult->value.boolean = 1;
}

void fx_String_prototype_indexOf(txMachine* the)
{
	txString aString;
	txString aSubString;
	txInteger aLength;
	txInteger aSubLength;
	txInteger anOffset;
	txNumber aNumber;
	txInteger aLimit;
	
	aString = fxCoerceToString(the, mxThis);
	if (mxArgc < 1) {
		mxResult->value.integer = -1;
		mxResult->kind = XS_INTEGER_KIND;
		return;
	}
	aSubString = fxToString(the, mxArgv(0));
	aLength = fxUnicodeLength(aString);
	aSubLength = fxUnicodeLength(aSubString);
	anOffset = 0;
	if ((mxArgc > 1) && (mxArgv(1)->kind != XS_UNDEFINED_KIND)) {
		aNumber = fxToNumber(the, mxArgv(1));
		anOffset = (c_isnan(aNumber)) ? 0 : (aNumber < 0) ? 0 : (aNumber > aLength) ? aLength : (txInteger)c_floor(aNumber);
	}
	if (anOffset + aSubLength <= aLength) {
		anOffset = fxUnicodeToUTF8Offset(aString, anOffset);
		aLimit = mxStringLength(aString) - mxStringLength(aSubString);
		while (anOffset <= aLimit) {
			txU1 c;
			txU1* p = (txU1*)aString + anOffset;
			txU1* q = (txU1*)aSubString;
			while ((c = c_read8(q)) && (c_read8(p) == c)) {
				mxMeterSome(((c & 0xC0) != 0x80) ? 1 : 0);
				p++;
				q++;
			}
			if (c)
				anOffset++;
			else
				break;
		}
		if (anOffset <= aLimit)
			anOffset = fxUTF8ToUnicodeOffset(aString, anOffset);
		else
			anOffset = -1;
	}
	else
		anOffset = -1;
	mxResult->value.integer = anOffset;
	mxResult->kind = XS_INTEGER_KIND;
}

static txInteger fx_String_prototype_indexOf_aux(txMachine* the, txString theString, 
		txInteger theLength, txInteger theOffset,
		txString theSubString, txInteger theSubLength, txInteger* theOffsets)
{
	theOffsets[0] = theOffset;
	theOffsets[1] = theOffset + theSubLength;
	while (theOffsets[1] <= theLength) {
		txU1 c;
		txU1* p = (txU1*)theString + theOffsets[0];
		txU1* q = (txU1*)theSubString;
		while ((c = c_read8(q)) && (c_read8(p) == c)) {
			mxMeterSome(((c & 0xC0) != 0x80) ? 1 : 0);
			p++;
			q++;
		}
		if (c) {
			theOffsets[0]++;
			theOffsets[1]++;
		}
		else
			return 1;
	}
	return 0;
}

void fx_String_prototype_lastIndexOf(txMachine* the)
{
	txString aString;
	txString aSubString;
	txInteger aLength;
	txInteger aSubLength;
	txInteger anOffset;
	txNumber aNumber;

	aString = fxCoerceToString(the, mxThis);
	if (mxArgc < 1) {
		mxResult->value.integer = -1;
		mxResult->kind = XS_INTEGER_KIND;
		return;
	}
	aSubString = fxToString(the, mxArgv(0));
	aLength = fxUnicodeLength(aString);
	aSubLength = fxUnicodeLength(aSubString);
	anOffset = aLength;
	if ((mxArgc > 1) && (mxArgv(1)->kind != XS_UNDEFINED_KIND)) {
		aNumber = fxToNumber(the, mxArgv(1));
		anOffset = (c_isnan(aNumber)) ? aLength : (aNumber < 0) ? 0 : (aNumber > aLength) ? aLength : (txInteger)c_floor(aNumber);
		anOffset += aSubLength;
		if (anOffset > aLength)
			anOffset = aLength;
	}
	if (anOffset - aSubLength >= 0) {
		anOffset = fxUnicodeToUTF8Offset(aString, anOffset - aSubLength);
		while (anOffset >= 0) {
			txU1 c;
			txU1* p = (txU1*)aString + anOffset;
			txU1* q = (txU1*)aSubString;
			while ((c = c_read8(q)) && (c_read8(p) == c)) {
				mxMeterSome(((c & 0xC0) != 0x80) ? 1 : 0);
				p++;
				q++;
			}
			if (c)
				anOffset--;
			else
				break;
		}		
		anOffset = fxUTF8ToUnicodeOffset(aString, anOffset);
	}
	else
		anOffset = -1;
	mxResult->value.integer = anOffset;
	mxResult->kind = XS_INTEGER_KIND;
}

void fx_String_prototype_localeCompare(txMachine* the)
{
	txString aString;
	txString bString;

	if (mxArgc < 1) {
		aString = fxCoerceToString(the, mxThis);
		bString = "undefined";
	}
	else {
		fxToString(the, mxArgv(0));
		aString = fxCoerceToString(the, mxThis);
		bString = mxArgv(0)->value.string;
	}
#ifdef mxMetering
	{
		txSize aLength = fxUnicodeLength(aString);
		txSize bLength = fxUnicodeLength(bString);
		if (aLength < bLength)
			the->meterIndex += aLength;
		else
			the->meterIndex += bLength;
	}
#endif	
	mxResult->value.integer = c_strcmp(aString, bString);
	mxResult->kind = XS_INTEGER_KIND;
}

void fx_String_prototype_match(txMachine* the)
{	
	if (fx_String_prototype_withRegexp(the, mxID(_Symbol_match), 0, 1))
		return;
	fx_String_prototype_withoutRegexp(the, mxID(_Symbol_match), 0, 1);
}

void fx_String_prototype_matchAll(txMachine* the)
{	
	if (fx_String_prototype_withRegexp(the, mxID(_Symbol_matchAll), 1, 1))
		return;
	fx_String_prototype_withoutRegexp(the, mxID(_Symbol_matchAll), 1, 1);
}

void fx_String_prototype_normalize(txMachine* the)
{
	txString result = mxEmptyString.value.string;
	#if (mxWindows && (WINVER >= 0x0600))
	txString string = fxCoerceToString(the, mxThis);
	txInteger stringLength = mxStringLength(string);
	mxMeterSome(fxUnicodeLength(string));
	{
		NORM_FORM form;
		txInteger unicodeLength;
		txU2* unicodeBuffer = NULL;
		txInteger normLength;
		txU2* normBuffer = NULL;
		txInteger resultLength;
		mxTry(the) {
			if ((mxArgc < 1) || (mxArgv(0)->kind == XS_UNDEFINED_KIND))
				form = NormalizationC;
			else {
				result = fxToString(the, mxArgv(0));
				if (!c_strcmp(result, "NFC"))
					form = NormalizationC;
				else if (!c_strcmp(result, "NFD"))
					form = NormalizationD;
				else if (!c_strcmp(result, "NFKC"))
					form = NormalizationKC;
				else if (!c_strcmp(result, "NFKD"))
					form = NormalizationKD;
				else
					mxRangeError("invalid form");
			}
			unicodeLength = MultiByteToWideChar(CP_UTF8, 0, string, stringLength, NULL, 0);
			unicodeBuffer = c_malloc(unicodeLength * 2);
			if (!unicodeBuffer) fxJump(the);
			MultiByteToWideChar(CP_UTF8, 0, string, stringLength, unicodeBuffer, unicodeLength);
			normLength = NormalizeString(form, unicodeBuffer, unicodeLength, NULL, 0);
			normBuffer = c_malloc(normLength * 2);
			if (!normBuffer) fxJump(the);
			NormalizeString(form, unicodeBuffer, unicodeLength, normBuffer, normLength);
			resultLength = WideCharToMultiByte(CP_UTF8, 0, normBuffer, normLength, NULL, 0, NULL, NULL);
			result = fxNewChunk(the, resultLength + 1);
			WideCharToMultiByte(CP_UTF8, 0, normBuffer, normLength, result, resultLength, NULL, NULL);
			result[resultLength] = 0;
			c_free(unicodeBuffer);
		}
		mxCatch(the) {
			if (unicodeBuffer)
				c_free(unicodeBuffer);
			fxJump(the);
		}
	}
	#elif (mxMacOSX || mxiOS)
	txString string = fxCoerceToString(the, mxThis);
	txInteger stringLength = mxStringLength(string);
	{
		CFStringNormalizationForm form;
		CFStringRef cfString = NULL;
		CFMutableStringRef mutableCFString = NULL;
		CFIndex resultLength;
		CFRange range;
		mxTry(the) {
			if ((mxArgc < 1) || (mxArgv(0)->kind == XS_UNDEFINED_KIND))
				form = kCFStringNormalizationFormC;
			else {
				result = fxToString(the, mxArgv(0));
				if (!c_strcmp(result, "NFC"))
					form = kCFStringNormalizationFormC;
				else if (!c_strcmp(result, "NFD"))
					form = kCFStringNormalizationFormD;
				else if (!c_strcmp(result, "NFKC"))
					form = kCFStringNormalizationFormKC;
				else if (!c_strcmp(result, "NFKD"))
					form = kCFStringNormalizationFormKD;
				else
					mxRangeError("invalid form");
			}
			cfString = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)string, stringLength, kCFStringEncodingUTF8, false);
			if (cfString == NULL) fxJump(the);
			mutableCFString = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, cfString);
			if (mutableCFString == NULL) fxJump(the);
			CFStringNormalize(mutableCFString, form);
			range = CFRangeMake(0, CFStringGetLength(mutableCFString));
			CFStringGetBytes(mutableCFString, range, kCFStringEncodingUTF8, 0, false, NULL, 0, &resultLength);
			result = fxNewChunk(the, resultLength + 1);
			CFStringGetBytes(mutableCFString, range, kCFStringEncodingUTF8, 0, false, (UInt8 *)result, resultLength, NULL);
			result[resultLength] = 0;
			CFRelease(mutableCFString);
			CFRelease(cfString);
		}
		mxCatch(the) {
			if (mutableCFString)
				CFRelease(mutableCFString);
			if (cfString)
				CFRelease(cfString);
			fxJump(the);
		}
	}
	#else
	{
		mxRangeError("invalid form");
	}
	#endif
	mxResult->value.string = result;
	mxResult->kind = XS_STRING_KIND;
}

void fx_String_prototype_pad(txMachine* the, txBoolean flag)
{
	txString string = fxCoerceToString(the, mxThis), filler;
	txInteger stringLength = mxStringLength(string), fillerLength;
	txInteger stringSize = fxUnicodeLength(string), fillerSize;
	txInteger resultSize = (txInteger)fxArgToRange(the, 0, 0, 0, 0x7FFFFFFF);
	*mxResult = *mxThis;
	if (resultSize > stringSize) {
		if ((mxArgc <= 1) || (mxIsUndefined(mxArgv(1))))
			mxPushStringX(" ");
		else
			mxPushSlot(mxArgv(1));
		filler = fxToString(the, the->stack);
		fillerLength = mxStringLength(filler);
		fillerSize = fxUnicodeLength(filler);
		if (fillerSize > 0) {
			txInteger delta = resultSize - stringSize;
			txInteger count = delta / fillerSize;
			txInteger rest = fxUnicodeToUTF8Offset(filler, delta % fillerSize);
			txString result = mxResult->value.string = (txString)fxNewChunk(the, fxAddChunkSizes(the, fxAddChunkSizes(the, stringLength, fxMultiplyChunkSizes(the, fillerLength, count)), rest + 1));
			mxResult->kind = XS_STRING_KIND;
			string = fxToString(the, mxThis);
			filler = fxToString(the, the->stack);
			if (flag) {
				c_memcpy(result, string, stringLength);
				result += stringLength;
			}
			mxMeterSome(count);
			while (count) {
				c_memcpy(result, filler, fillerLength);
				count--;
				result += fillerLength;
			}
			if (rest) {
				mxMeterSome(1);
				c_memcpy(result, filler, rest);
				result += rest;
			}
			if (!flag) {
				c_memcpy(result, string, stringLength);
				result += stringLength;
			}
			*result = 0;
		}
		mxPop();
	}
}

void fx_String_prototype_padEnd(txMachine* the)
{
	fx_String_prototype_pad(the, 1);
}

void fx_String_prototype_padStart(txMachine* the)
{
	fx_String_prototype_pad(the, 0);
}

void fx_String_prototype_repeat(txMachine* the)
{
	txString string = fxCoerceToString(the, mxThis), result;
	txInteger length = mxStringLength(string);
	txInteger count = 0;
	txSlot *arg = mxArgv(0);
	if ((mxArgc > 0) && (arg->kind != XS_UNDEFINED_KIND)) {
		if (XS_INTEGER_KIND == arg->kind) {
			count = arg->value.integer;
			if (count < 0)
				mxRangeError("out of range count");
		}
		else {
			txNumber value = c_trunc(fxToNumber(the, arg));
			if (c_isnan(value))
				count = 0;
			else {
				if ((value < 0) || (0x7FFFFFFF < value))
					mxRangeError("out of range count");
				count = (txInteger)value;
			}
		}
	}
	mxMeterSome(count);
	result = mxResult->value.string = (txString)fxNewChunk(the, fxAddChunkSizes(the, fxMultiplyChunkSizes(the, length, count), 1));
	mxResult->kind = XS_STRING_KIND;
	string = fxToString(the, mxThis);
	if (length) {
		while (count) {
			c_memcpy(result, string, length);
			count--;
			result += length;
		}
	}
	*result = 0;
	string = mxThis->value.string;
}

void fx_String_prototype_replace(txMachine* the)
{
	txString string;
	txSlot* match;
	txSize size;
	txSize matchLength;
	txSlot* function = C_NULL;
	txSlot* replace;

	if (fx_String_prototype_withRegexp(the, mxID(_Symbol_replace), 0, 2))
		return;
	string = fxToString(the, mxThis);
	if (mxArgc <= 0)
		mxPushUndefined();
	else
		mxPushSlot(mxArgv(0));
	match = the->stack;
	fxToString(the, match);
	if (mxArgc <= 1)
		mxPushUndefined();
	else
		mxPushSlot(mxArgv(1));
	if (mxIsReference(the->stack) && mxIsFunction(the->stack->value.reference))
		function = the->stack;
	else {		
		replace = the->stack;
		fxToString(the, replace);
	}
	size = mxStringLength(mxThis->value.string);
	matchLength = mxStringLength(match->value.string);
	string = fx_String_prototype_includes_aux(the, mxThis->value.string, size, match->value.string, matchLength);
	if (string) {
		txSize offset = mxPtrDiff(string - mxThis->value.string);
		txSize replaceLength;
 		fx_String_prototype_replaceAux(the, size, offset, function, match, matchLength, replace);
		replaceLength = mxStringLength(the->stack->value.string);
		mxResult->value.string = (txString)fxNewChunk(the, size - matchLength + replaceLength + 1);
		c_memcpy(mxResult->value.string, mxThis->value.string, offset);
		c_memcpy(mxResult->value.string + offset, the->stack->value.string, replaceLength);
		c_memcpy(mxResult->value.string + offset + replaceLength, mxThis->value.string + offset + matchLength, size - (offset + matchLength));
		mxResult->value.string[size - matchLength + replaceLength] = 0;
		mxResult->kind = XS_STRING_KIND;
		mxPop();
	}
	else
		*mxResult = *mxThis;
	mxPop();
	mxPop();
}

void fx_String_prototype_replaceAll(txMachine* the)
{
	txSlot* match;
	txSlot* function = C_NULL;
	txSlot* replace;
	txInteger size;
	txInteger matchLength;
	txInteger resultSize = 0;
	txSlot* list;
	txSlot* item;
	txInteger offset = 0;

	if (fx_String_prototype_withRegexp(the, mxID(_Symbol_replace), 1, 2))
		return;
	fxToString(the, mxThis);
	if (mxArgc <= 0)
		mxPushUndefined();
	else
		mxPushSlot(mxArgv(0));
	match = the->stack;
	fxToString(the, match);
	if (mxArgc <= 1)
		mxPushUndefined();
	else
		mxPushSlot(mxArgv(1));
	if (mxIsReference(the->stack) && mxIsFunction(the->stack->value.reference))
		function = the->stack;
	else {		
		replace = the->stack;
		fxToString(the, replace);
	}
	size = mxStringLength(mxThis->value.string);
	matchLength = mxStringLength(match->value.string);
	
	list = item = fxNewInstance(the);
	mxPushSlot(list);
	
	if (!matchLength) {
		fx_String_prototype_replaceAux(the, size, offset, function, match, matchLength, replace);
		item = item->next = fxNewSlot(the);
		mxPullSlot(item);
		item->value.key.sum = mxStringLength(item->value.string);
		resultSize += item->value.key.sum;
	}
	while (offset < size) {
		txInteger current;
		if (!matchLength) {
			current = offset + fxUnicodeToUTF8Offset(mxThis->value.string + offset, 1);
		}
		else {
			txString string = fx_String_prototype_includes_aux(the, mxThis->value.string + offset, size - offset, match->value.string, matchLength);
            if (string)
				current = mxPtrDiff(string - mxThis->value.string);
			else
				current = size;
		}
		if (offset < current) {
			txInteger length = current - offset;
			item = item->next = fxNewSlot(the);
			item->value.string = (txString)fxNewChunk(the, length + 1);
			c_memcpy(item->value.string, mxThis->value.string + offset, length);
			item->value.string[length] = 0;
			item->kind = XS_STRING_KIND;
			item->value.key.sum = length;
			resultSize += length;
		}
		if ((!matchLength) || (current < size)) {
 			fx_String_prototype_replaceAux(the, size, current, function, match, matchLength, replace);
			item = item->next = fxNewSlot(the);
            mxPullSlot(item);
			item->value.key.sum = mxStringLength(item->value.string);
			resultSize += item->value.key.sum;
		}
		offset = current + matchLength;
	}		
	resultSize++;
	mxResult->value.string = (txString)fxNewChunk(the, resultSize);
	offset = 0;
	item = list->next;
	while (item) {
		c_memcpy(mxResult->value.string + offset, item->value.string, item->value.key.sum);
		offset += item->value.key.sum;
		item = item->next;
	}
	mxResult->value.string[offset] = 0;
	mxResult->kind = XS_STRING_KIND;
	
	mxPop();
	
	mxPop();
	mxPop();
}

void fx_String_prototype_replaceAux(txMachine* the, txInteger size, txInteger offset, txSlot* function, txSlot* match, txInteger matchLength, txSlot* replace)
{
	if (function) {
		mxPushUndefined();
		mxPushSlot(function);
		mxCall();
		mxPushSlot(match);
		mxPushInteger(fxUTF8ToUnicodeOffset(mxThis->value.string, offset));
		mxPushSlot(mxThis);
		mxRunCount(3);
		fxToString(the, the->stack);
	}
	else
		fxPushSubstitutionString(the, mxThis, size, offset, match, matchLength, 0, C_NULL, C_NULL, replace);
}

void fx_String_prototype_search(txMachine* the)
{
	if (fx_String_prototype_withRegexp(the, mxID(_Symbol_search), 0, 1))
		return;
	fx_String_prototype_withoutRegexp(the, mxID(_Symbol_search), 0, 1);
}

void fx_String_prototype_slice(txMachine* the)
{
	txString string = fxCoerceToString(the, mxThis);
	txInteger length = fxUnicodeLength(string);
	txNumber start = fxArgToIndex(the, 0, 0, length);
	txNumber end = fxArgToIndex(the, 1, length, length);
	if (start < end) {
		txInteger offset = fxUnicodeToUTF8Offset(string, (txInteger)start);
		length = fxUnicodeToUTF8Offset(string + offset, (txInteger)(end - start));
		if ((offset >= 0) && (length > 0)) {
			mxResult->value.string = (txString)fxNewChunk(the, length + 1);
			c_memcpy(mxResult->value.string, mxThis->value.string + offset, length);
			mxResult->value.string[length] = 0;
			mxResult->kind = XS_STRING_KIND;
			return;
		}
	}
	mxResult->value.string = mxEmptyString.value.string;
	mxResult->kind = mxEmptyString.kind;
}

void fx_String_prototype_split(txMachine* the)
{
	txString aString;
	txInteger aLength;
	txIndex aLimit;
	txSlot* anArray;
	txSlot* anItem;
	txInteger anOffset;
	txInteger aCount;
	txString aSubString;
	txInteger aSubLength;
	txInteger aSubOffset;
	txInteger subOffsets[2];

	if (fx_String_prototype_withRegexp(the, mxID(_Symbol_split), 0, 2))
		return;
	aString = fxToString(the, mxThis);
	aLength = mxStringLength(aString);
	aLimit = ((mxArgc > 1) && (!mxIsUndefined(mxArgv(1)))) ? (txIndex)fxToUnsigned(the, mxArgv(1)) : 0xFFFFFFFF;
	mxPush(mxArrayPrototype);
	anArray = fxNewArrayInstance(the);
	mxPullSlot(mxResult);
	fxGetInstance(the, mxResult);
	anItem = fxLastProperty(the, anArray);
	if ((mxArgc < 1) || (mxArgv(0)->kind == XS_UNDEFINED_KIND)) {
		if (!aLimit)
			goto bail;
		fx_String_prototype_split_aux(the, mxThis, anArray, anItem, 0, aLength);
		goto bail;
	}
	aSubString = fxToString(the, mxArgv(0));
	aSubLength = mxStringLength(aSubString);
	if (!aLimit)
		goto bail;
	if (aSubLength == 0) {
		anOffset = 0;
		while (anOffset < aLength) {
			aSubOffset = anOffset + fxUnicodeToUTF8Offset(mxThis->value.string + anOffset, 1);
			anItem = fx_String_prototype_split_aux(the, mxThis, anArray, anItem, anOffset, aSubOffset);
			if (anArray->next->value.array.length >= aLimit)
				goto bail;
			anOffset = aSubOffset;
		}
	}
	else if (aLength == 0) {
		fx_String_prototype_split_aux(the, mxThis, anArray, anItem, 0, 0);
	}
	else {
		anOffset = 0;
		for (;;) {
			aCount = fx_String_prototype_indexOf_aux(the, mxThis->value.string, aLength, anOffset, mxArgv(0)->value.string, aSubLength, subOffsets);
			if (aCount <= 0)
				break;
			if (anOffset <= subOffsets[0]) {
				anItem = fx_String_prototype_split_aux(the, mxThis, anArray, anItem, anOffset, subOffsets[0]);
				if (anArray->next->value.array.length >= aLimit)
					goto bail;
			}
			anOffset = subOffsets[1];
		}
		if (anOffset <= aLength)
			fx_String_prototype_split_aux(the, mxThis, anArray, anItem, anOffset, aLength);
	}
bail:
	fxCacheArray(the, anArray);
}

txSlot* fx_String_prototype_split_aux(txMachine* the, txSlot* theString, txSlot* theArray, txSlot* theItem, txInteger theStart, txInteger theStop)
{
	theStop -= theStart;
	theItem->next = fxNewSlot(the);
	theItem = theItem->next;
	theItem->next = C_NULL;
	if (theStart >= 0) {
		theItem->value.string = (txString)fxNewChunk(the, theStop + 1);
		c_memcpy(theItem->value.string, theString->value.string + theStart, theStop);
		theItem->value.string[theStop] = 0;
		theItem->kind = XS_STRING_KIND;
	}
	theArray->next->value.array.length++;
	return theItem;
}

void fx_String_prototype_startsWith(txMachine* the)
{
	txString string = fxCoerceToString(the, mxThis);
	txInteger length = mxStringLength(string);
	txString searchString;
	txInteger searchLength;
	txInteger offset;
	mxResult->kind = XS_BOOLEAN_KIND;
	mxResult->value.boolean = 0;
	if (mxArgc < 1)
		return;
	if (fxIsRegExp(the, mxArgv(0)))
		mxTypeError("future editions");
	searchString = fxToString(the, mxArgv(0));
	searchLength = mxStringLength(searchString);
	offset = fxUnicodeToUTF8Offset(string, fxArgToPosition(the, 1, 0, fxUnicodeLength(string)));
	if (length - offset < searchLength)
		return;
	mxMeterSome(fxUnicodeLength(searchString));
	if (!c_strncmp(string + offset, searchString, searchLength))
		mxResult->value.boolean = 1;
}

void fx_String_prototype_substr(txMachine* the)
{
	txString string = fxCoerceToString(the, mxThis);
	txInteger size = fxUnicodeLength(string);
	txInteger start = (txInteger)fxArgToIndex(the, 0, 0, size);
	txInteger stop = size;
	if ((mxArgc > 1) && (mxArgv(1)->kind != XS_UNDEFINED_KIND)) {
		stop = start + fxToInteger(the, mxArgv(1));
		if (stop > size)
			stop = size;
	}	
	if (start < stop) {
		txInteger length;
		start = fxUnicodeToUTF8Offset(string, start);
		stop = fxUnicodeToUTF8Offset(string, stop);
		length = stop - start;
		mxResult->value.string = (txString)fxNewChunk(the, length + 1);
		c_memcpy(mxResult->value.string, string + start, length);
		mxResult->value.string[length] = 0;
		mxResult->kind = XS_STRING_KIND;
	}
	else {
		mxResult->value.string = mxEmptyString.value.string;
		mxResult->kind = mxEmptyString.kind;
	}
}

void fx_String_prototype_substring(txMachine* the)
{
	txString aString;
	txInteger aLength;
	txNumber aNumber;
	txInteger aStart;
	txInteger aStop;
	txInteger anOffset;

	aString = fxCoerceToString(the, mxThis);
	aLength = fxUnicodeLength(aString);
	aStart = 0;
	aStop = aLength;
	if ((mxArgc > 0) && (mxArgv(0)->kind != XS_UNDEFINED_KIND)) {
		aNumber = fxToNumber(the, mxArgv(0));
		aStart = (c_isnan(aNumber)) ? 0 : (aNumber < 0) ? 0 : (aNumber > aLength) ? aLength : (txInteger)c_floor(aNumber);
	}
	if ((mxArgc > 1) && (mxArgv(1)->kind != XS_UNDEFINED_KIND)) {
		aNumber = fxToNumber(the, mxArgv(1));
		aStop = (c_isnan(aNumber)) ? 0 : (aNumber < 0) ? 0 : (aNumber > aLength) ? aLength : (txInteger)c_floor(aNumber);
	}
	if (aStart > aStop) {
		aLength = aStart;
		aStart = aStop;
		aStop = aLength;
	}
	if (aStart < aStop) {
		anOffset = fxUnicodeToUTF8Offset(aString, aStart);
		aLength = fxUnicodeToUTF8Offset(aString + anOffset, aStop - aStart);
		if ((anOffset >= 0) && (aLength > 0)) {
			mxResult->value.string = (txString)fxNewChunk(the, aLength + 1);
			c_memcpy(mxResult->value.string, mxThis->value.string + anOffset, aLength);
			mxResult->value.string[aLength] = 0;
			mxResult->kind = XS_STRING_KIND;
		}
		else {
			mxResult->value.string = mxEmptyString.value.string;
			mxResult->kind = mxEmptyString.kind;
		}
	}
	else {
		mxResult->value.string = mxEmptyString.value.string;
		mxResult->kind = mxEmptyString.kind;
	}
}

void fx_String_prototype_toCase(txMachine* the, txBoolean flag)
{
	txString string = fxCoerceToString(the, mxThis);
	txInteger stringLength = mxStringLength(string);
	mxMeterSome(fxUnicodeLength(string));
	if (stringLength) {
	#if mxWindows
		txInteger unicodeLength;
		txU2* unicodeBuffer = NULL;
		txInteger resultLength;
		txString result;
		mxTry(the) {
			unicodeLength = MultiByteToWideChar(CP_UTF8, 0, string, stringLength, NULL, 0);
			if (unicodeLength == 0) fxJump(the);
			unicodeBuffer = c_malloc(unicodeLength * 2);
			if (unicodeBuffer == NULL) fxJump(the);
			MultiByteToWideChar(CP_UTF8, 0, string, stringLength, unicodeBuffer, unicodeLength);
			if (flag)
				CharUpperBuffW(unicodeBuffer, unicodeLength);
			else
				CharLowerBuffW(unicodeBuffer, unicodeLength);
			resultLength = WideCharToMultiByte(CP_UTF8, 0, unicodeBuffer, unicodeLength, NULL, 0, NULL, NULL);
			result = fxNewChunk(the, resultLength + 1);
			WideCharToMultiByte(CP_UTF8, 0, unicodeBuffer, unicodeLength, result, resultLength, NULL, NULL);
			result[resultLength] = 0;
			c_free(unicodeBuffer);
			mxResult->value.string = result;
			mxResult->kind = XS_STRING_KIND;
		}
		mxCatch(the) {
			if (unicodeBuffer)
				c_free(unicodeBuffer);
			fxJump(the);
		}
	#elif (mxMacOSX || mxiOS)
		CFStringRef cfString = NULL;
		CFMutableStringRef mutableCFString = NULL;
		CFRange range;
		CFIndex resultLength;
		txString result;
		mxTry(the) {
			cfString = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)string, stringLength, kCFStringEncodingUTF8, false);
			if (cfString == NULL) fxJump(the);
			mutableCFString = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, cfString);
			if (mutableCFString == NULL) fxJump(the);
			if (flag)
				CFStringUppercase(mutableCFString, 0);
			else
				CFStringLowercase(mutableCFString, 0);
			range = CFRangeMake(0, CFStringGetLength(mutableCFString));
			CFStringGetBytes(mutableCFString, range, kCFStringEncodingUTF8, 0, false, NULL, 0, &resultLength);
			result = fxNewChunk(the, resultLength + 1);
			CFStringGetBytes(mutableCFString, range, kCFStringEncodingUTF8, 0, false, (UInt8 *)result, resultLength, NULL);
			result[resultLength] = 0;
			CFRelease(mutableCFString);
			CFRelease(cfString);
			mxResult->value.string = result;
			mxResult->kind = XS_STRING_KIND;
		}
		mxCatch(the) {
			if (mutableCFString)
				CFRelease(mutableCFString);
			if (cfString)
				CFRelease(cfString);
			fxJump(the);
		}
	#else
		txString s, r;
		txInteger c;
		const txCharCase* current;
		const txCharCase* limit = flag ? &gxCharCaseToUpper[mxCharCaseToUpperCount] : &gxCharCaseToLower[mxCharCaseToLowerCount];
		mxResult->value.string = fxNewChunk(the, stringLength + 1);
		mxResult->kind = XS_STRING_KIND;
		s = mxThis->value.string;
		r = mxResult->value.string;
		while (((s = fxUTF8Decode(s, &c))) && (c != C_EOF)) {
			current = flag ? gxCharCaseToUpper : gxCharCaseToLower;
			while (current < limit) {
				if (c < current->from)
					break;
				if (c <= current->to) {
					if (current->delta)
						c += current->delta;
					else if (flag)
						c &= ~1;
					else
						c |= 1;
					break;
				}
				current++;
			}
			r = fxUTF8Encode(r, c);
		}
		*r = 0;
	#endif
	}
	else {
		mxResult->value.string = mxEmptyString.value.string;
		mxResult->kind = mxEmptyString.kind;
	}
}

void fx_String_prototype_toLowerCase(txMachine* the)
{
	fx_String_prototype_toCase(the, 0);
}

void fx_String_prototype_toUpperCase(txMachine* the)
{
	fx_String_prototype_toCase(the, 1);
}

void fx_String_prototype_trim(txMachine* the)
{
	fx_String_prototype_trimAux(the, 1, 1);
}

void fx_String_prototype_trimAux(txMachine* the, txBoolean trimStart, txBoolean trimEnd)
{
	txString string = fxCoerceToString(the, mxThis), start;
	txInteger offset, length;
	if (trimStart) {
		start = fxSkipSpaces(string);
		offset = mxPtrDiff(start - string);
		mxMeterSome(offset);
	}
	else {
		start = string;
		offset = 0;
	}
	if (trimEnd) {
		txString current = start;
		txString end = current;
		while (c_read8(current)) {
			end = current + 1;
			current = fxSkipSpaces(end);
		}
		length = mxPtrDiff(end - start);
		mxMeterSome(length);
	}
	else
		length = mxStringLength(start);
	mxResult->value.string = (txString)fxNewChunk(the, length + 1);
	c_memcpy(mxResult->value.string, mxThis->value.string + offset, length);
	mxResult->value.string[length] = 0;
	mxResult->kind = XS_STRING_KIND;
}

void fx_String_prototype_trimEnd(txMachine* the)
{
	fx_String_prototype_trimAux(the, 0, 1);
}

void fx_String_prototype_trimStart(txMachine* the)
{
	fx_String_prototype_trimAux(the, 1, 0);
}

void fx_String_prototype_valueOf(txMachine* the)
{
	txSlot* slot = fxCheckString(the, mxThis);
	if (!slot) mxTypeError("this is no string");
	mxResult->kind = slot->kind;
	mxResult->value = slot->value;
}

txBoolean fx_String_prototype_withRegexp(txMachine* the, txID id, txBoolean global, txInteger count)
{
	if (mxIsUndefined(mxThis))
		mxTypeError("this is undefined");
	if (mxIsNull(mxThis))
		mxTypeError("this is null");
	if (mxArgc > 0) {
		txSlot* regexp = mxArgv(0);
		if (!mxIsUndefined(regexp) && !mxIsNull(regexp)) {
			if (global && fxIsRegExp(the, regexp)) {
				mxPushSlot(regexp);
				mxGetID(mxID(_flags));
				if (!c_strchr(fxToString(the, the->stack), 'g'))
					mxTypeError("regexp has no g flag");
				mxPop();
			}
			mxPushSlot(regexp);
			mxPushSlot(regexp);
			mxGetID(id);
			if (!mxIsUndefined(the->stack) && !mxIsNull(the->stack)) {
				mxCall();
				mxPushSlot(mxThis);
				if (count > 1) {
					if (mxArgc > 1)
						mxPushSlot(mxArgv(1));
					else
						mxPushUndefined();
				}
				mxRunCount(count);
				mxPullSlot(mxResult);
				return 1;
			}
			mxPop();
			mxPop();
		}
	}
	return 0;
}

void fx_String_prototype_withoutRegexp(txMachine* the, txID id, txBoolean global, txInteger count)
{
	fxToString(the, mxThis);
	mxPush(mxRegExpPrototype);
	fxNewRegExpInstance(the);
	mxPush(mxInitializeRegExpFunction);
	mxCall();
	if (mxArgc > 0)
		mxPushSlot(mxArgv(0));
	else
		mxPushUndefined();
	if (global)
		mxPushStringX("g");
	else
		mxPushUndefined();
	mxRunCount(2);
	mxDub();
	mxGetID(id);
	mxCall();
	mxPushSlot(mxThis);
	if (count > 1) {
		if (mxArgc > 1)
			mxPushSlot(mxArgv(1));
		else
			mxPushUndefined();
	}
	mxRunCount(count);
	mxPullSlot(mxResult);
}

txSlot* fxCheckString(txMachine* the, txSlot* it)
{
	txSlot* result = C_NULL;
	if ((it->kind == XS_STRING_KIND) || (it->kind == XS_STRING_X_KIND))
		result = it;
	else if (it->kind == XS_REFERENCE_KIND) {
		it = it->value.reference->next;
		if (it && (it->flag & XS_INTERNAL_FLAG) && ((it->kind == XS_STRING_KIND) || (it->kind == XS_STRING_X_KIND)))
			result = it;
	}
	return result;
}

txString fxCoerceToString(txMachine* the, txSlot* theSlot)
{
	if (theSlot->kind == XS_UNDEFINED_KIND)
		mxTypeError("this is undefined");
	if (theSlot->kind == XS_NULL_KIND)
		mxTypeError("this is null");
	return fxToString(the, theSlot);
}

void fx_String_prototype_iterator(txMachine* the)
{
	txString string = fxCoerceToString(the, mxThis);
	txSlot* property;
	mxPush(mxStringIteratorPrototype);
	property = fxLastProperty(the, fxNewIteratorInstance(the, mxThis, mxID(_String)));
	property = fxNextIntegerProperty(the, property, fxUnicodeLength(string), XS_NO_ID, XS_INTERNAL_FLAG);
	mxPullSlot(mxResult);
}

void fx_String_prototype_iterator_next(txMachine* the)
{
	txSlot* iterator = fxCheckIteratorInstance(the, mxThis, mxID(_String));
	txSlot* result = iterator->next;
	txSlot* iterable = result->next;
	txSlot* index = iterable->next;
	txSlot* length = index->next;
	txSlot* value = result->value.reference->next;
	txSlot* done = value->next;
	if (index->value.integer < length->value.integer) {
		txInteger offset = fxUnicodeToUTF8Offset(iterable->value.string, index->value.integer);
		txInteger length = fxUnicodeToUTF8Offset(iterable->value.string + offset, 1);
		value->value.string = (txString)fxNewChunk(the, length + 1);
		c_memcpy(value->value.string, iterable->value.string + offset, length);
		value->value.string[length] = 0;
		value->kind = XS_STRING_KIND;
		index->value.integer++;
	}
	else {
		value->kind = XS_UNDEFINED_KIND;
		done->value.boolean = 1;
	}
	mxResult->kind = result->kind;
	mxResult->value = result->value;
}

txInteger fxArgToPosition(txMachine* the, txInteger argi, txInteger index, txInteger length)
{
	if ((mxArgc > argi) && (mxArgv(argi)->kind != XS_UNDEFINED_KIND)) {
		txNumber i = c_trunc(fxToNumber(the, mxArgv(argi)));
		if (c_isnan(i))
			i = 0;
		if (i < 0)
			index = 0;
		else if (i > (txNumber)length)
			index = length;
		else
			index = (txInteger)i;
	}
	return index;
}

void fxPushSubstitutionString(txMachine* the, txSlot* string, txInteger size, txInteger offset, txSlot* match, txInteger length, txInteger count, txSlot* captures, txSlot* groups, txSlot* replace)
{
	txString r;
	txInteger l;
	txBoolean flag;
	txByte c, d;
	txInteger i, j;
	txSlot* capture;
	txString s;
	r = replace->value.string;
	l = 0;
	flag = 0;
	while ((c = c_read8(r++))) {
		if (c == '$') {
			c = c_read8(r++);
			switch (c) {
			case '$':
				l++;
				flag = 1;
				break;
			case '&':
				l += length;
				flag = 1;
				break;
			case '`':
				l += offset;
				flag = 1;
				break;
			case '\'':
				l += size - (offset + length);
				flag = 1;
				break;
			case '<':
				if (groups && mxIsReference(groups)) {
					txString t = r;
					while ((d = c_read8(r))) {
						if (d == '>')
							break;
						r++;
					}
					if (d) {
						txInteger n = mxPtrDiff(r - t);
						txID name;
						if (n > 255)
							fxJump(the);
						c_memcpy(the->nameBuffer, t, n);
						the->nameBuffer[n] = 0;
						name = fxFindName(the, the->nameBuffer);
						if (name) {
 							mxPushSlot(groups);
							mxGetID(name);
							if (!mxIsUndefined(the->stack)) {
								fxToString(the, the->stack);
								l += mxStringLength(the->stack->value.string);
							}
							mxPop();
						}
						r++;
						flag = 1;
					}
					else {
						r = t;
						l += 2;
					}
				}
				else {
					l += 2;
				}
				break;
			default:
				if (('0' <= c) && (c <= '9')) {
					i = c - '0';
					d = c_read8(r);
					if (('0' <= d) && (d <= '9')) {
						j = (i * 10) + d - '0';
						if ((0 < j) && (j <= count)) {
							i = j;
							r++;
						}
						else
							d = 0;
					}
					else
						d = 0;
					if ((0 < i) && (i <= count)) {
						capture = (captures + count - i);
						if (capture->kind != XS_UNDEFINED_KIND)
							l += mxStringLength(capture->value.string);
						flag = 1;
					}
					else {
						l++;
						l++;
						if (d)
							l++;
					}
				}
				else {
					l++;
					if (c)
						l++;
				}
				break;
			}
			if (!c)
				break;
		}
		else
			l++;
	}
	if (flag) {
		mxPushUndefined();
		the->stack->value.string = (txString)fxNewChunk(the, l + 1);
		the->stack->kind = XS_STRING_KIND;
		r = replace->value.string;
		s = the->stack->value.string;
		while ((c = c_read8(r++))) {
			if (c == '$') {
				c = c_read8(r++);
				switch (c) {
				case '$':
					*s++ = c;
					break;
				case '&':
					l = length;
					c_memcpy(s, match->value.string, l);
					s += l;
					break;
				case '`':
					l = offset;
					c_memcpy(s, string->value.string, l);
					s += l;
					break;
				case '\'':
					l = size - (offset + length);
                    if (l > 0) {
                        c_memcpy(s, string->value.string + offset + length, l);
                        s += l;
                    }
					break;
				case '<':
					if (groups && mxIsReference(groups)) {
						txString t = r;
						while ((d = c_read8(r))) {
							if (d == '>')
								break;
							r++;
						}
						if (d) {
							txInteger n = mxPtrDiff(r - t);
							txID name;
							if (n > 255)
								fxJump(the);
							c_memcpy(the->nameBuffer, t, n);
							the->nameBuffer[n] = 0;
							name = fxFindName(the, the->nameBuffer);
							if (name) {
								mxPushSlot(groups);
								mxGetID(name);
								if (!mxIsUndefined(the->stack)) {
									fxToString(the, the->stack);
									l = mxStringLength(the->stack->value.string);
									c_memcpy(s, the->stack->value.string, l);
									s += l;
								}
								mxPop();
							}
							r++;
						}
						else {
							r = t;
							*s++ = '$';
							*s++ = '<';
						}
					}
					else {
						*s++ = '$';
						*s++ = '<';
					}
					break;
				default:
					if (('0' <= c) && (c <= '9')) {
						i = c - '0';
						d = c_read8(r);
						if (('0' <= d) && (d <= '9')) {
							j = (i * 10) + d - '0';
							if ((0 < j) && (j <= count)) {
								i = j;
								r++;
							}
							else
								d = 0;
						}
						else
							d = 0;
						if ((0 < i) && (i <= count)) {
							capture = (captures + count - i);
							if (capture->kind != XS_UNDEFINED_KIND) {
								l = mxStringLength(capture->value.string);
								c_memcpy(s, capture->value.string, l);
								s += l;
							}
						}
						else {
							*s++ = '$';
							*s++ = c;
							if (d)
								*s++ = d;
						}
					}
					else {
						*s++ = '$';
						if (c)
							*s++ = c;
					}
					break;
				}
				if (!c)
					break;
			}
			else
				*s++ = c;
		}
		*s = 0;
	}
	else
		mxPushSlot(replace);
}

