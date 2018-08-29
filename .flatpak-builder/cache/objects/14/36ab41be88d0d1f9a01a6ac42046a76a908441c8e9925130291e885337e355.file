/*
 * Copyright 2018 Lucian Poston
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "config.h"
#include "wine/port.h"

#include "d2d1_private.h"

WINE_DEFAULT_DEBUG_CHANNEL(d2d);

static inline struct d2d_device *impl_from_ID2D1Device(ID2D1Device *iface)
{
    return CONTAINING_RECORD(iface, struct d2d_device, ID2D1Device_iface);
}

static HRESULT WINAPI d2d_device_QueryInterface(
        ID2D1Device *iface,
        REFIID riid,
        void **ppvObject)
{
    TRACE("iface %p, riid %s, ppvObject %p.\n", iface, debugstr_guid(riid), ppvObject);
    if (ppvObject == NULL)
        return E_POINTER;

    if (IsEqualGUID(riid, &IID_ID2D1Device)
            || IsEqualGUID(riid, &IID_ID2D1Resource)
            || IsEqualGUID(riid, &IID_IUnknown))
    {
        ID2D1Device_AddRef(iface);
        *ppvObject = iface;
        return S_OK;
    }

    WARN("%s not implemented, returning E_NOINTERFACE.\n", debugstr_guid(riid));
    *ppvObject = NULL;
    return E_NOINTERFACE;
}

static ULONG WINAPI d2d_device_AddRef(
        ID2D1Device *iface)
{
    struct d2d_device *This = impl_from_ID2D1Device(iface);
    ULONG refcount = InterlockedIncrement(&This->refcount);
    TRACE("%p increasing refcount to %u.\n", iface, refcount);
    return refcount;
}

static ULONG WINAPI d2d_device_Release(
        ID2D1Device *iface)
{
    struct d2d_device *This = impl_from_ID2D1Device(iface);
    ULONG refcount = InterlockedDecrement(&This->refcount);
    TRACE("%p decreasing refcount to %u.\n", iface, refcount);

    if (refcount == 0)
    {
        IDXGIDevice_Release(This->dxgi_device);
        ID2D1Factory1_Release(This->factory);
        HeapFree(GetProcessHeap(), 0, This);
    }

    return refcount;
}

static void WINAPI d2d_device_GetFactory(
        ID2D1Device *iface,
        ID2D1Factory **factory)
{
    struct d2d_device *This = impl_from_ID2D1Device(iface);

    TRACE("iface %p, factory %p.\n", iface, factory);
    *factory = (ID2D1Factory *)This->factory;
    ID2D1Factory1_AddRef(This->factory);
}

static HRESULT WINAPI d2d_device_CreateDeviceContext(
        ID2D1Device *iface,
        D2D1_DEVICE_CONTEXT_OPTIONS options,
        ID2D1DeviceContext **deviceContext)
{
    struct d2d_device *This = impl_from_ID2D1Device(iface);
    struct d2d_device_context *object;
    ID3D10Device *d3d_device;
    HRESULT hr;

    TRACE("This %p, options %#x\n", This, options);
    if (deviceContext == NULL)
        return E_POINTER;

    if (FAILED(hr = IDXGIDevice_QueryInterface(This->dxgi_device,
                    &IID_ID3D10Device, (void **)&d3d_device)))
    {
        WARN("Failed to query d3d device, hr %#x.\n", hr);
        return hr;
    }

    if (!(object = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*object))))
    {
        ID3D10Device_Release(d3d_device);
        return E_OUTOFMEMORY;
    }

    hr = d2d_device_context_init(object, iface, options, d3d_device);
    ID3D10Device_Release(d3d_device);
    if (FAILED(hr))
    {
        HeapFree(GetProcessHeap(), 0, object);
        WARN("Failed to create device context, hr %#x.\n", hr);
        return hr;
    }

    *deviceContext = &object->ID2D1DeviceContext_iface;
    TRACE("Created device context %p.\n", object);

    return S_OK;
}

static HRESULT WINAPI d2d_device_CreatePrintControl(
        ID2D1Device *iface,
        IWICImagingFactory *wicFactory,
        IPrintDocumentPackageTarget *documentTarget,
        const D2D1_PRINT_CONTROL_PROPERTIES *printControlProperties,
        ID2D1PrintControl **printControl)
{
    struct d2d_device *This = impl_from_ID2D1Device(iface);
    FIXME("%p stub!\n", This);
    return E_NOTIMPL;
}

static void WINAPI d2d_device_SetMaximumTextureMemory(
        ID2D1Device *iface,
        UINT64 maximumInBytes)
{
    struct d2d_device *This = impl_from_ID2D1Device(iface);
    FIXME("%p stub!\n", This);
}

static UINT64 WINAPI d2d_device_GetMaximumTextureMemory(
        ID2D1Device *iface)
{
    struct d2d_device *This = impl_from_ID2D1Device(iface);
    FIXME("%p stub!\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI d2d_device_ClearResources(
        ID2D1Device *iface,
        UINT millisecondsSinceUse)
{
    struct d2d_device *This = impl_from_ID2D1Device(iface);
    FIXME("%p stub!\n", This);
    return E_NOTIMPL;
}

static const struct ID2D1DeviceVtbl d2d_device_vtbl =
{
    d2d_device_QueryInterface,
    d2d_device_AddRef,
    d2d_device_Release,
    d2d_device_GetFactory,
    d2d_device_CreateDeviceContext,
    d2d_device_CreatePrintControl,
    d2d_device_SetMaximumTextureMemory,
    d2d_device_GetMaximumTextureMemory,
    d2d_device_ClearResources,
};

void d2d_device_init(struct d2d_device *device, ID2D1Factory1 *iface, IDXGIDevice *dxgi_device)
{
    device->ID2D1Device_iface.lpVtbl = &d2d_device_vtbl;
    device->refcount = 1;
    device->factory = iface;
    ID2D1Factory1_AddRef(device->factory);
    device->dxgi_device = dxgi_device;
    IDXGIDevice_AddRef(device->dxgi_device);
}
