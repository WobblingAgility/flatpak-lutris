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

static inline struct d2d_device_context *impl_from_ID2D1DeviceContext(ID2D1DeviceContext *iface)
{
    return CONTAINING_RECORD(iface, struct d2d_device_context, ID2D1DeviceContext_iface);
}

static HRESULT WINAPI d2d_device_context_QueryInterface(
        ID2D1DeviceContext *iface,
        REFIID riid,
        void **ppvObject)
{
    TRACE("iface %p, riid %s, ppvObject %p.\n", iface, debugstr_guid(riid), ppvObject);
    if (ppvObject == NULL)
        return E_POINTER;

    if (IsEqualGUID(riid, &IID_ID2D1DeviceContext)
            || IsEqualGUID(riid, &IID_ID2D1RenderTarget)
            || IsEqualGUID(riid, &IID_ID2D1Resource)
            || IsEqualGUID(riid, &IID_IUnknown))
    {
        ID2D1DeviceContext_AddRef(iface);
        *ppvObject = iface;
        return S_OK;
    }

    WARN("%s not implemented, returning E_NOINTERFACE.\n", debugstr_guid(riid));
    *ppvObject = NULL;
    return E_NOINTERFACE;
}

static ULONG WINAPI d2d_device_context_AddRef(
        ID2D1DeviceContext *iface)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    ULONG refcount = InterlockedIncrement(&This->refcount);
    TRACE("%p increasing refcount to %u.\n", iface, refcount);
    return refcount;
}

static ULONG WINAPI d2d_device_context_Release(
        ID2D1DeviceContext *iface)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    ULONG refcount = InterlockedDecrement(&This->refcount);
    TRACE("%p decreasing refcount to %u.\n", iface, refcount);

    if (refcount == 0)
    {
        ID2D1RenderTarget_Release(This->dxgi_target);
        ID2D1Device_Release(This->device);
        HeapFree(GetProcessHeap(), 0, This);
    }

    return refcount;
}

static void WINAPI d2d_device_context_GetFactory(
        ID2D1DeviceContext *iface,
        ID2D1Factory **factory)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, factory %p.\n", This, factory);
    ID2D1Device_GetFactory(This->device, factory);
}

static HRESULT WINAPI d2d_device_context_CreateBitmap(
        ID2D1DeviceContext *iface,
        D2D1_SIZE_U size,
        const void *src_data,
        UINT32 pitch,
        const D2D1_BITMAP_PROPERTIES *desc,
        ID2D1Bitmap **bitmap)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, src_data %p, desc %p, bitmap %p.\n", This, src_data, desc, bitmap);
    return ID2D1RenderTarget_CreateBitmap(This->dxgi_target, size, src_data, pitch, desc, bitmap);
}

static HRESULT WINAPI d2d_device_context_CreateBitmapFromWicBitmap(
        ID2D1DeviceContext *iface,
        IWICBitmapSource *bitmap_source,
        const D2D1_BITMAP_PROPERTIES *desc,
        ID2D1Bitmap **bitmap)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, bitmap_source %p, desc %p, bitmap %p.\n",
            This, bitmap_source, desc, bitmap);
    return ID2D1RenderTarget_CreateBitmapFromWicBitmap(This->dxgi_target, bitmap_source, desc, bitmap);
}

static HRESULT WINAPI d2d_device_context_CreateSharedBitmap(
        ID2D1DeviceContext *iface,
        REFIID iid,
        void *data,
        const D2D1_BITMAP_PROPERTIES *desc,
        ID2D1Bitmap **bitmap)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, iid %s, data %p, desc %p, bitmap %p.\n",
            This, debugstr_guid(iid), data, desc, bitmap);
    return ID2D1RenderTarget_CreateSharedBitmap(This->dxgi_target, iid, data, desc, bitmap);
}

static HRESULT WINAPI d2d_device_context_CreateBitmapBrush(
        ID2D1DeviceContext *iface,
        ID2D1Bitmap *bitmap,
        const D2D1_BITMAP_BRUSH_PROPERTIES *bitmap_brush_desc,
        const D2D1_BRUSH_PROPERTIES *brush_desc,
        ID2D1BitmapBrush **brush)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, bitmap %p, bitmap_brush_desc %p, brush_desc %p, brush %p.\n",
            This, bitmap, bitmap_brush_desc, brush_desc, brush);
    return ID2D1RenderTarget_CreateBitmapBrush(This->dxgi_target,
            bitmap, bitmap_brush_desc, brush_desc, brush);
}

static HRESULT WINAPI d2d_device_context_CreateSolidColorBrush(
        ID2D1DeviceContext *iface,
        const D2D1_COLOR_F *color,
        const D2D1_BRUSH_PROPERTIES *desc,
        ID2D1SolidColorBrush **brush)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, color %p, desc %p, brush %p.\n", This, color, desc, brush);
    return ID2D1RenderTarget_CreateSolidColorBrush(This->dxgi_target, color, desc, brush);
}

static HRESULT WINAPI d2d_device_context_CreateGradientStopCollection(
        ID2D1DeviceContext *iface,
        const D2D1_GRADIENT_STOP *stops,
        UINT32 stop_count,
        D2D1_GAMMA gamma,
        D2D1_EXTEND_MODE extend_mode,
        ID2D1GradientStopCollection **gradient)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, stops %p, gradient %p.\n", This, stops, gradient);
    return ID2D1RenderTarget_CreateGradientStopCollection(This->dxgi_target,
            stops, stop_count, gamma, extend_mode, gradient);
}

static HRESULT WINAPI d2d_device_context_CreateLinearGradientBrush(
        ID2D1DeviceContext *iface,
        const D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES *gradient_brush_desc,
        const D2D1_BRUSH_PROPERTIES *brush_desc,
        ID2D1GradientStopCollection *gradient,
        ID2D1LinearGradientBrush **brush)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, gradient_brush_desc %p, brush_desc %p, gradient %p, brush %p.\n",
            This, gradient_brush_desc, brush_desc, gradient, brush);
    return ID2D1RenderTarget_CreateLinearGradientBrush(This->dxgi_target,
            gradient_brush_desc, brush_desc, gradient, brush);
}

static HRESULT WINAPI d2d_device_context_CreateRadialGradientBrush(
        ID2D1DeviceContext *iface,
        const D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES *gradient_brush_desc,
        const D2D1_BRUSH_PROPERTIES *brush_desc,
        ID2D1GradientStopCollection *gradient,
        ID2D1RadialGradientBrush **brush)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, gradient_brush_desc %p, brush_desc %p, gradient %p, brush %p.\n",
            This, gradient_brush_desc, brush_desc, gradient, brush);
    return ID2D1RenderTarget_CreateRadialGradientBrush(This->dxgi_target,
            gradient_brush_desc, brush_desc, gradient, brush);
}

static HRESULT WINAPI d2d_device_context_CreateCompatibleRenderTarget(
        ID2D1DeviceContext *iface,
        const D2D1_SIZE_F *size,
        const D2D1_SIZE_U *pixel_size,
        const D2D1_PIXEL_FORMAT *format,
        D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS options,
        ID2D1BitmapRenderTarget **render_target)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, size %p, pixel_size %p, format %p, render_target %p.\n",
            This, size, pixel_size, format, render_target);
    return ID2D1RenderTarget_CreateCompatibleRenderTarget(This->dxgi_target,
            size, pixel_size, format, options, render_target);
}

static HRESULT WINAPI d2d_device_context_CreateLayer(
        ID2D1DeviceContext *iface,
        const D2D1_SIZE_F *size,
        ID2D1Layer **layer)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, size %p, layer %p.\n", This, size, layer);
    return ID2D1RenderTarget_CreateLayer(This->dxgi_target, size, layer);
}

static HRESULT WINAPI d2d_device_context_CreateMesh(
        ID2D1DeviceContext *iface,
        ID2D1Mesh **mesh)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, mesh %p.\n", This, mesh);
    return ID2D1RenderTarget_CreateMesh(This->dxgi_target, mesh);
}

static void WINAPI d2d_device_context_DrawLine(
        ID2D1DeviceContext *iface,
        D2D1_POINT_2F p0,
        D2D1_POINT_2F p1,
        ID2D1Brush *brush,
        float stroke_width,
        ID2D1StrokeStyle *stroke_style)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, brush %p, stroke_style %p.\n", This, brush, stroke_style);
    ID2D1RenderTarget_DrawLine(This->dxgi_target, p0, p1, brush, stroke_width, stroke_style);
}

static void WINAPI d2d_device_context_DrawRectangle(
        ID2D1DeviceContext *iface,
        const D2D1_RECT_F *rect,
        ID2D1Brush *brush,
        float stroke_width,
        ID2D1StrokeStyle *stroke_style)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, rect %p, brush %p, stroke_style %p.\n", This, rect, brush, stroke_style);
    ID2D1RenderTarget_DrawRectangle(This->dxgi_target, rect, brush, stroke_width, stroke_style);
}

static void WINAPI d2d_device_context_FillRectangle(
        ID2D1DeviceContext *iface,
        const D2D1_RECT_F *rect,
        ID2D1Brush *brush)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, rect %p, brush %p.\n", This, rect, brush);
    ID2D1RenderTarget_FillRectangle(This->dxgi_target, rect, brush);
}

static void WINAPI d2d_device_context_DrawRoundedRectangle(
        ID2D1DeviceContext *iface,
        const D2D1_ROUNDED_RECT *rect,
        ID2D1Brush *brush,
        float stroke_width,
        ID2D1StrokeStyle *stroke_style)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, rect %p, brush %p, stroke_style %p.\n", This, rect, brush, stroke_style);
    ID2D1RenderTarget_DrawRoundedRectangle(This->dxgi_target, rect, brush, stroke_width, stroke_style);
}

static void WINAPI d2d_device_context_FillRoundedRectangle(
        ID2D1DeviceContext *iface,
        const D2D1_ROUNDED_RECT *rect,
        ID2D1Brush *brush)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, rect %p, brush %p.\n", This, rect, brush);
    ID2D1RenderTarget_FillRoundedRectangle(This->dxgi_target, rect, brush);
}

static void WINAPI d2d_device_context_DrawEllipse(
        ID2D1DeviceContext *iface,
        const D2D1_ELLIPSE *ellipse,
        ID2D1Brush *brush,
        float stroke_width,
        ID2D1StrokeStyle *stroke_style)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, ellipse %p, brush %p, stroke_style %p.\n", This, ellipse, brush, stroke_style);
    ID2D1RenderTarget_DrawEllipse(This->dxgi_target, ellipse, brush, stroke_width, stroke_style);
}

static void WINAPI d2d_device_context_FillEllipse(
        ID2D1DeviceContext *iface,
        const D2D1_ELLIPSE *ellipse,
        ID2D1Brush *brush)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, ellipse %p, brush %p.\n", This, ellipse, brush);
    ID2D1RenderTarget_FillEllipse(This->dxgi_target, ellipse, brush);
}

static void WINAPI d2d_device_context_DrawGeometry(
        ID2D1DeviceContext *iface,
        ID2D1Geometry *geometry,
        ID2D1Brush *brush,
        float stroke_width,
        ID2D1StrokeStyle *stroke_style)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, geometry %p, brush %p, stroke_style %p.\n", This, geometry, brush, stroke_style);
    ID2D1RenderTarget_DrawGeometry(This->dxgi_target, geometry, brush, stroke_width, stroke_style);
}

static void WINAPI d2d_device_context_FillGeometry(
        ID2D1DeviceContext *iface,
        ID2D1Geometry *geometry,
        ID2D1Brush *brush,
        ID2D1Brush *opacity_brush)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, geometry %p, brush %p, opacity_brush %p.\n", This, geometry, brush, opacity_brush);
    ID2D1RenderTarget_FillGeometry(This->dxgi_target, geometry, brush, opacity_brush);
}

static void WINAPI d2d_device_context_FillMesh(
        ID2D1DeviceContext *iface,
        ID2D1Mesh *mesh,
        ID2D1Brush *brush)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, mesh %p, brush %p.\n", This, mesh, brush);
    ID2D1RenderTarget_FillMesh(This->dxgi_target, mesh, brush);
}

static void WINAPI d2d_device_context_FillOpacityMask(
        ID2D1DeviceContext *iface,
        ID2D1Bitmap *mask,
        ID2D1Brush *brush,
        D2D1_OPACITY_MASK_CONTENT content,
        const D2D1_RECT_F *dst_rect,
        const D2D1_RECT_F *src_rect)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, mask %p, brush %p.\n", This, mask, brush);
    ID2D1RenderTarget_FillOpacityMask(This->dxgi_target,
            mask, brush, content, dst_rect, src_rect);
}

static void WINAPI d2d_device_context_DrawBitmap(
        ID2D1DeviceContext *iface,
        ID2D1Bitmap *bitmap,
        const D2D1_RECT_F *dst_rect,
        float opacity,
        D2D1_BITMAP_INTERPOLATION_MODE interpolation_mode,
        const D2D1_RECT_F *src_rect)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, bitmap %p.\n", This, bitmap);
    ID2D1RenderTarget_DrawBitmap(This->dxgi_target,
            bitmap, dst_rect, opacity, interpolation_mode, src_rect);
}

static void WINAPI d2d_device_context_DrawText(
        ID2D1DeviceContext *iface,
        const WCHAR *string,
        UINT32 string_len,
        IDWriteTextFormat *text_format,
        const D2D1_RECT_F *layout_rect,
        ID2D1Brush *brush,
        D2D1_DRAW_TEXT_OPTIONS options,
        DWRITE_MEASURING_MODE measuring_mode)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, string %s.\n", This, debugstr_w(string));
    ID2D1RenderTarget_DrawText(This->dxgi_target, string, string_len,
            text_format, layout_rect, brush, options, measuring_mode);
}

static void WINAPI d2d_device_context_DrawTextLayout(
        ID2D1DeviceContext *iface,
        D2D1_POINT_2F origin,
        IDWriteTextLayout *layout,
        ID2D1Brush *brush,
        D2D1_DRAW_TEXT_OPTIONS options)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, layout %p, brush %p.\n", This, layout, brush);
    ID2D1RenderTarget_DrawTextLayout(This->dxgi_target, origin, layout, brush, options);
}

static void WINAPI d2d_device_context_DrawGlyphRun(
        ID2D1DeviceContext *iface,
        D2D1_POINT_2F baseline_origin,
        const DWRITE_GLYPH_RUN *glyph_run,
        ID2D1Brush *brush,
        DWRITE_MEASURING_MODE measuring_mode)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, glyph_run %p, brush %p.\n", This, glyph_run, brush);
    ID2D1RenderTarget_DrawGlyphRun(This->dxgi_target,
            baseline_origin, glyph_run, brush, measuring_mode);
}

static void WINAPI d2d_device_context_SetTransform(
        ID2D1DeviceContext *iface,
        const D2D1_MATRIX_3X2_F *transform)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, transform %p.\n", This, transform);
    ID2D1RenderTarget_SetTransform(This->dxgi_target, transform);
}

static void WINAPI d2d_device_context_GetTransform(
        ID2D1DeviceContext *iface,
        D2D1_MATRIX_3X2_F *transform)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, transform %p.\n", This, transform);
    ID2D1RenderTarget_GetTransform(This->dxgi_target, transform);
}

static void WINAPI d2d_device_context_SetAntialiasMode(
        ID2D1DeviceContext *iface,
        D2D1_ANTIALIAS_MODE antialias_mode)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p.\n", This);
    ID2D1RenderTarget_SetAntialiasMode(This->dxgi_target, antialias_mode);
}

static D2D1_ANTIALIAS_MODE WINAPI d2d_device_context_GetAntialiasMode(
        ID2D1DeviceContext *iface)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p.\n", This);
    return ID2D1RenderTarget_GetAntialiasMode(This->dxgi_target);
}

static void WINAPI d2d_device_context_SetTextAntialiasMode(
        ID2D1DeviceContext *iface,
        D2D1_TEXT_ANTIALIAS_MODE antialias_mode)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p.\n", This);
    ID2D1RenderTarget_SetTextAntialiasMode(This->dxgi_target, antialias_mode);
}

static D2D1_TEXT_ANTIALIAS_MODE WINAPI d2d_device_context_GetTextAntialiasMode(
        ID2D1DeviceContext *iface)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p.\n", This);
    return ID2D1RenderTarget_GetTextAntialiasMode(This->dxgi_target);
}

static void WINAPI d2d_device_context_SetTextRenderingParams(
        ID2D1DeviceContext *iface,
        IDWriteRenderingParams *text_rendering_params)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p.\n", This);
    ID2D1RenderTarget_SetTextRenderingParams(This->dxgi_target, text_rendering_params);
}

static void WINAPI d2d_device_context_GetTextRenderingParams(
        ID2D1DeviceContext *iface,
        IDWriteRenderingParams **text_rendering_params)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p.\n", This);
    ID2D1RenderTarget_GetTextRenderingParams(This->dxgi_target, text_rendering_params);
}

static void WINAPI d2d_device_context_SetTags(
        ID2D1DeviceContext *iface,
        D2D1_TAG tag1,
        D2D1_TAG tag2)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p.\n", This);
    ID2D1RenderTarget_SetTags(This->dxgi_target, tag1, tag2);
}

static void WINAPI d2d_device_context_GetTags(
        ID2D1DeviceContext *iface,
        D2D1_TAG *tag1,
        D2D1_TAG *tag2)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p.\n", This);
    ID2D1RenderTarget_GetTags(This->dxgi_target, tag1, tag2);
}

static void WINAPI d2d_device_context_PushLayer(
        ID2D1DeviceContext *iface,
        const D2D1_LAYER_PARAMETERS *layer_parameters,
        ID2D1Layer *layer)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p.\n", This);
    ID2D1RenderTarget_PushLayer(This->dxgi_target, layer_parameters, layer);
}

static void WINAPI d2d_device_context_PopLayer(
        ID2D1DeviceContext *iface)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p.\n", This);
    ID2D1RenderTarget_PopLayer(This->dxgi_target);
}

static HRESULT WINAPI d2d_device_context_Flush(
        ID2D1DeviceContext *iface,
        D2D1_TAG *tag1,
        D2D1_TAG *tag2)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p.\n", This);
    return ID2D1RenderTarget_Flush(This->dxgi_target, tag1, tag2);
}

static void WINAPI d2d_device_context_SaveDrawingState(
        ID2D1DeviceContext *iface,
        ID2D1DrawingStateBlock *state_block)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, state_block %p.\n", This, state_block);
    ID2D1RenderTarget_SaveDrawingState(This->dxgi_target, state_block);
}

static void WINAPI d2d_device_context_RestoreDrawingState(
        ID2D1DeviceContext *iface,
        ID2D1DrawingStateBlock *state_block)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, state_block %p.\n", This, state_block);
    ID2D1RenderTarget_RestoreDrawingState(This->dxgi_target, state_block);
}

static void WINAPI d2d_device_context_PushAxisAlignedClip(
        ID2D1DeviceContext *iface,
        const D2D1_RECT_F *clip_rect,
        D2D1_ANTIALIAS_MODE antialias_mode)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p.\n", This);
    ID2D1RenderTarget_PushAxisAlignedClip(This->dxgi_target, clip_rect, antialias_mode);
}

static void WINAPI d2d_device_context_PopAxisAlignedClip(
        ID2D1DeviceContext *iface)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p.\n", This);
    ID2D1RenderTarget_PopAxisAlignedClip(This->dxgi_target);
}

static void WINAPI d2d_device_context_Clear(
        ID2D1DeviceContext *iface,
        const D2D1_COLOR_F *color)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p.\n", This);
    ID2D1RenderTarget_Clear(This->dxgi_target, color);
}

static void WINAPI d2d_device_context_BeginDraw(
        ID2D1DeviceContext *iface)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p.\n", This);
    ID2D1RenderTarget_BeginDraw(This->dxgi_target);
}

static HRESULT WINAPI d2d_device_context_EndDraw(
        ID2D1DeviceContext *iface,
        D2D1_TAG *tag1,
        D2D1_TAG *tag2)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p.\n", This);
    return ID2D1RenderTarget_EndDraw(This->dxgi_target, tag1, tag2);
}

static D2D1_PIXEL_FORMAT * WINAPI d2d_device_context_GetPixelFormat(
        ID2D1DeviceContext *iface,
        D2D1_PIXEL_FORMAT *__ret)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, __ret %p.\n", This, __ret);
    *__ret = ID2D1RenderTarget_GetPixelFormat(This->dxgi_target);
    return __ret;
}

static void WINAPI d2d_device_context_SetDpi(
        ID2D1DeviceContext *iface,
        float dpi_x,
        float dpi_y)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p.\n", This);
    ID2D1RenderTarget_SetDpi(This->dxgi_target, dpi_x, dpi_y);
}

static void WINAPI d2d_device_context_GetDpi(
        ID2D1DeviceContext *iface,
        float *dpi_x,
        float *dpi_y)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p.\n", This);
    ID2D1RenderTarget_GetDpi(This->dxgi_target, dpi_x, dpi_y);
}

static D2D1_SIZE_F * WINAPI d2d_device_context_GetSize(
        ID2D1DeviceContext *iface,
        D2D1_SIZE_F *__ret)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, __ret %p.\n", This, __ret);
    *__ret = ID2D1RenderTarget_GetSize(This->dxgi_target);
    return __ret;
}

static D2D1_SIZE_U * WINAPI d2d_device_context_GetPixelSize(
        ID2D1DeviceContext *iface,
        D2D1_SIZE_U *__ret)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, __ret %p.\n", This, __ret);
    *__ret = ID2D1RenderTarget_GetPixelSize(This->dxgi_target);
    return __ret;
}

static UINT32 WINAPI d2d_device_context_GetMaximumBitmapSize(
        ID2D1DeviceContext *iface)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p.\n", This);
    return ID2D1RenderTarget_GetMaximumBitmapSize(This->dxgi_target);
}

static BOOL WINAPI d2d_device_context_IsSupported(
        ID2D1DeviceContext *iface,
        const D2D1_RENDER_TARGET_PROPERTIES *desc)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p.\n", This);
    return ID2D1RenderTarget_IsSupported(This->dxgi_target, desc);
}

static HRESULT WINAPI d2d_device_context_ID2D1DeviceContext_CreateBitmap(
        ID2D1DeviceContext *iface,
        D2D1_SIZE_U size,
        const void *srcData,
        UINT32 pitch,
        const D2D1_BITMAP_PROPERTIES1 *bitmapProperties,
        ID2D1Bitmap1 **bitmap)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI d2d_device_context_ID2D1DeviceContext_CreateBitmapFromWicBitmap(
        ID2D1DeviceContext *iface,
        IWICBitmapSource *wicBitmapSource,
        const D2D1_BITMAP_PROPERTIES1 *bitmapProperties,
        ID2D1Bitmap1 **bitmap)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI d2d_device_context_CreateColorContext(
        ID2D1DeviceContext *iface,
        D2D1_COLOR_SPACE space,
        const BYTE *Profile,
        UINT32 profileSize,
        ID2D1ColorContext **colorContext)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI d2d_device_context_CreateColorContextFromFilename(
        ID2D1DeviceContext *iface,
        PCWSTR Filename,
        ID2D1ColorContext **colorContext)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI d2d_device_context_CreateColorContextFromWicColorContext(
        ID2D1DeviceContext *iface,
        IWICColorContext *wicColorContext,
        ID2D1ColorContext **colorContext)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI d2d_device_context_CreateBitmapFromDxgiSurface(
        ID2D1DeviceContext *iface,
        IDXGISurface *surface,
        const D2D1_BITMAP_PROPERTIES1 *bitmapProperties,
        ID2D1Bitmap1 **bitmap)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    struct d2d_bitmap *bitmap_impl;
    HRESULT hr;
    ID2D1Factory *factory;

    TRACE("This %p, surface %p, bitmapProperties %p, bitmap %p.\n",
            This, surface, bitmapProperties, bitmap);
    if (surface == NULL || bitmap == NULL)
        return E_POINTER;

    ID2D1Device_GetFactory(This->device, &factory);
    hr = d2d_bitmap_create_shared_from_dxgi_surface(factory, surface,
            bitmapProperties, NULL, &bitmap_impl);
    ID2D1Factory_Release(factory);
    if (FAILED(hr))
    {
        WARN("Failed to create bitmap, hr %#x.\n", hr);
        return hr;
    }

    *bitmap = &bitmap_impl->ID2D1Bitmap_iface;

    return S_OK;
}

static HRESULT WINAPI d2d_device_context_CreateEffect(
        ID2D1DeviceContext *iface,
        REFCLSID effectId,
        ID2D1Effect **effect)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI d2d_device_context_ID2D1DeviceContext_CreateGradientStopCollection(
        ID2D1DeviceContext *iface,
        const D2D1_GRADIENT_STOP *gradientStops,
        UINT gradientStopsCount,
        D2D1_COLOR_SPACE preInterpolationSpace,
        D2D1_COLOR_SPACE postInterpolationSpace,
        D2D1_BUFFER_PRECISION bufferPrecision,
        D2D1_EXTEND_MODE extendMode,
        D2D1_COLOR_INTERPOLATION_MODE colorInterpolationMode,
        ID2D1GradientStopCollection1 **gradientStopCollection)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI d2d_device_context_CreateImageBrush(
        ID2D1DeviceContext *iface,
        ID2D1Image *image,
        const D2D1_IMAGE_BRUSH_PROPERTIES *imageBrushProperties,
        const D2D1_BRUSH_PROPERTIES *brushProperties,
        ID2D1ImageBrush **imageBrush)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI d2d_device_context_ID2D1DeviceContext_CreateBitmapBrush(
        ID2D1DeviceContext *iface,
        ID2D1Bitmap *bitmap,
        const D2D1_BITMAP_BRUSH_PROPERTIES1 *bitmapBrushProperties,
        const D2D1_BRUSH_PROPERTIES *brushProperties,
        ID2D1BitmapBrush1 **bitmapBrush)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI d2d_device_context_CreateCommandList(
        ID2D1DeviceContext *iface,
        ID2D1CommandList **commandList)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
    return E_NOTIMPL;
}

static BOOL WINAPI d2d_device_context_IsDxgiFormatSupported(
        ID2D1DeviceContext *iface,
        DXGI_FORMAT format)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
    return FALSE;
}

static BOOL WINAPI d2d_device_context_IsBufferPrecisionSupported(
        ID2D1DeviceContext *iface,
        D2D1_BUFFER_PRECISION bufferPrecision)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
    return FALSE;
}

static void WINAPI d2d_device_context_GetImageLocalBounds(
        ID2D1DeviceContext *iface,
        ID2D1Image *image,
        D2D1_RECT_F *localBounds)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
}

static HRESULT WINAPI d2d_device_context_GetImageWorldBounds(
        ID2D1DeviceContext *iface,
        ID2D1Image *image,
        D2D1_RECT_F *worldBounds)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI d2d_device_context_GetGlyphRunWorldBounds(
        ID2D1DeviceContext *iface,
        D2D1_POINT_2F baselineOrigin,
        const DWRITE_GLYPH_RUN *glyphRun,
        DWRITE_MEASURING_MODE measuringMode,
        D2D1_RECT_F *bounds)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
    return E_NOTIMPL;
}

static void WINAPI d2d_device_context_GetDevice(
        ID2D1DeviceContext *iface,
        ID2D1Device **device)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    TRACE("This %p, device %p.\n", This, device);
    if (device == NULL)
        return;

    ID2D1Device_AddRef(This->device);
    *device = This->device;
}

static void WINAPI d2d_device_context_SetTarget(
        ID2D1DeviceContext *iface,
        ID2D1Image *target)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    IDXGISurface *surface;
    IDXGISurface1 *surface1;
    ID2D1Bitmap1 *bitmap;
    HRESULT hr;

    TRACE("This %p, target %p.\n", This, target);
    if (FAILED(hr = ID2D1Image_QueryInterface(target, &IID_ID2D1Bitmap1, (void **)&bitmap)))
    {
        FIXME("Provided ID2D1Image type not yet supported, hr %#x.\n", hr);
        return;
    }

    ID2D1Bitmap1_GetSurface(bitmap, &surface);
    ID2D1Bitmap1_Release(bitmap);
    hr = IDXGISurface_QueryInterface(surface, &IID_IDXGISurface1, (void **)&surface1);
    IDXGISurface_Release(surface);
    if (FAILED(hr))
    {
        WARN("Failed to query IDXGISurface1, hr %#x.\n", hr);
        return;
    }

    if (FAILED(d2d_d3d_render_target_create_rtv(This->dxgi_target, surface1)))
    {
        WARN("Failed to set renderviewtarget, hr %#x.\n", hr);
    }

    IDXGISurface1_Release(surface1);
}

static void WINAPI d2d_device_context_GetTarget(
        ID2D1DeviceContext *iface,
        ID2D1Image **target)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
}

static void WINAPI d2d_device_context_SetRenderingControls(
        ID2D1DeviceContext *iface,
        const D2D1_RENDERING_CONTROLS *renderingControls)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
}

static void WINAPI d2d_device_context_GetRenderingControls(
        ID2D1DeviceContext *iface,
        D2D1_RENDERING_CONTROLS *renderingControls)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
}

static void WINAPI d2d_device_context_SetPrimitiveBlend(
        ID2D1DeviceContext *iface,
        D2D1_PRIMITIVE_BLEND primitiveBlend)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
}

static D2D1_PRIMITIVE_BLEND WINAPI d2d_device_context_GetPrimitiveBlend(
        ID2D1DeviceContext *iface)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
    return D2D1_PRIMITIVE_BLEND_SOURCE_OVER;
}

static void WINAPI d2d_device_context_SetUnitMode(
        ID2D1DeviceContext *iface,
        D2D1_UNIT_MODE unitMode)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
}

static D2D1_UNIT_MODE WINAPI d2d_device_context_GetUnitMode(
        ID2D1DeviceContext *iface)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
    return D2D1_UNIT_MODE_DIPS;
}

static void WINAPI d2d_device_context_ID2D1DeviceContext_DrawGlyphRun(
        ID2D1DeviceContext *iface,
        D2D1_POINT_2F baselineOrigin,
        const DWRITE_GLYPH_RUN *glyphRun,
        const DWRITE_GLYPH_RUN_DESCRIPTION *glyphRunDescription,
        ID2D1Brush *foregroundBrush,
        DWRITE_MEASURING_MODE measuringMode)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
}

static void WINAPI d2d_device_context_DrawImage(
        ID2D1DeviceContext *iface,
        ID2D1Image *image,
        const D2D1_POINT_2F *targetOffset,
        const D2D1_RECT_F *imageRectangle,
        D2D1_INTERPOLATION_MODE interpolationMode,
        D2D1_COMPOSITE_MODE compositeMode)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
}

static void WINAPI d2d_device_context_DrawGdiMetafile(
        ID2D1DeviceContext *iface,
        ID2D1GdiMetafile *gdiMetafile,
        const D2D1_POINT_2F *targetOffset)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
}

static void WINAPI d2d_device_context_ID2D1DeviceContext_DrawBitmap(
        ID2D1DeviceContext *iface,
        ID2D1Bitmap *bitmap,
        const D2D1_RECT_F *destinationRectangle,
        float opacity,
        D2D1_INTERPOLATION_MODE interpolationMode,
        const D2D1_RECT_F *sourceRectangle,
        const D2D1_MATRIX_4X4_F *perspectiveTransform)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
}

static void WINAPI d2d_device_context_ID2D1DeviceContext_PushLayer(
        ID2D1DeviceContext *iface,
        const D2D1_LAYER_PARAMETERS1 *layerParameters,
        ID2D1Layer *layer)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
}

static HRESULT WINAPI d2d_device_context_InvalidateEffectInputRectangle(
        ID2D1DeviceContext *iface,
        ID2D1Effect *effect,
        UINT32 input,
        const D2D1_RECT_F *inputRectangle)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI d2d_device_context_GetEffectInvalidRectangleCount(
        ID2D1DeviceContext *iface,
        ID2D1Effect *effect,
        UINT32 *rectangleCount)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI d2d_device_context_GetEffectInvalidRectangles(
        ID2D1DeviceContext *iface,
        ID2D1Effect *effect,
        D2D1_RECT_F *rectangles,
        UINT32 rectanglesCount)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI d2d_device_context_GetEffectRequiredInputRectangles(
        ID2D1DeviceContext *iface,
        ID2D1Effect *renderEffect,
        const D2D1_RECT_F *renderImageRectangle,
        const D2D1_EFFECT_INPUT_DESCRIPTION *inputDescriptions,
        D2D1_RECT_F *requiredInputRects,
        UINT32 inputCount)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
    return E_NOTIMPL;
}

static void WINAPI d2d_device_context_ID2D1DeviceContext_FillOpacityMask(
        ID2D1DeviceContext *iface,
        ID2D1Bitmap *opacityMask,
        ID2D1Brush *brush,
        const D2D1_RECT_F *destinationRectangle,
        const D2D1_RECT_F *sourceRectangle)
{
    struct d2d_device_context *This = impl_from_ID2D1DeviceContext(iface);
    FIXME("%p stub!\n", This);
}

static const struct ID2D1DeviceContextVtbl d2d_device_context_vtbl =
{
    d2d_device_context_QueryInterface,
    d2d_device_context_AddRef,
    d2d_device_context_Release,
    d2d_device_context_GetFactory,
    d2d_device_context_CreateBitmap,
    d2d_device_context_CreateBitmapFromWicBitmap,
    d2d_device_context_CreateSharedBitmap,
    d2d_device_context_CreateBitmapBrush,
    d2d_device_context_CreateSolidColorBrush,
    d2d_device_context_CreateGradientStopCollection,
    d2d_device_context_CreateLinearGradientBrush,
    d2d_device_context_CreateRadialGradientBrush,
    d2d_device_context_CreateCompatibleRenderTarget,
    d2d_device_context_CreateLayer,
    d2d_device_context_CreateMesh,
    d2d_device_context_DrawLine,
    d2d_device_context_DrawRectangle,
    d2d_device_context_FillRectangle,
    d2d_device_context_DrawRoundedRectangle,
    d2d_device_context_FillRoundedRectangle,
    d2d_device_context_DrawEllipse,
    d2d_device_context_FillEllipse,
    d2d_device_context_DrawGeometry,
    d2d_device_context_FillGeometry,
    d2d_device_context_FillMesh,
    d2d_device_context_FillOpacityMask,
    d2d_device_context_DrawBitmap,
    d2d_device_context_DrawText,
    d2d_device_context_DrawTextLayout,
    d2d_device_context_DrawGlyphRun,
    d2d_device_context_SetTransform,
    d2d_device_context_GetTransform,
    d2d_device_context_SetAntialiasMode,
    d2d_device_context_GetAntialiasMode,
    d2d_device_context_SetTextAntialiasMode,
    d2d_device_context_GetTextAntialiasMode,
    d2d_device_context_SetTextRenderingParams,
    d2d_device_context_GetTextRenderingParams,
    d2d_device_context_SetTags,
    d2d_device_context_GetTags,
    d2d_device_context_PushLayer,
    d2d_device_context_PopLayer,
    d2d_device_context_Flush,
    d2d_device_context_SaveDrawingState,
    d2d_device_context_RestoreDrawingState,
    d2d_device_context_PushAxisAlignedClip,
    d2d_device_context_PopAxisAlignedClip,
    d2d_device_context_Clear,
    d2d_device_context_BeginDraw,
    d2d_device_context_EndDraw,
    d2d_device_context_GetPixelFormat,
    d2d_device_context_SetDpi,
    d2d_device_context_GetDpi,
    d2d_device_context_GetSize,
    d2d_device_context_GetPixelSize,
    d2d_device_context_GetMaximumBitmapSize,
    d2d_device_context_IsSupported,
    d2d_device_context_ID2D1DeviceContext_CreateBitmap,
    d2d_device_context_ID2D1DeviceContext_CreateBitmapFromWicBitmap,
    d2d_device_context_CreateColorContext,
    d2d_device_context_CreateColorContextFromFilename,
    d2d_device_context_CreateColorContextFromWicColorContext,
    d2d_device_context_CreateBitmapFromDxgiSurface,
    d2d_device_context_CreateEffect,
    d2d_device_context_ID2D1DeviceContext_CreateGradientStopCollection,
    d2d_device_context_CreateImageBrush,
    d2d_device_context_ID2D1DeviceContext_CreateBitmapBrush,
    d2d_device_context_CreateCommandList,
    d2d_device_context_IsDxgiFormatSupported,
    d2d_device_context_IsBufferPrecisionSupported,
    d2d_device_context_GetImageLocalBounds,
    d2d_device_context_GetImageWorldBounds,
    d2d_device_context_GetGlyphRunWorldBounds,
    d2d_device_context_GetDevice,
    d2d_device_context_SetTarget,
    d2d_device_context_GetTarget,
    d2d_device_context_SetRenderingControls,
    d2d_device_context_GetRenderingControls,
    d2d_device_context_SetPrimitiveBlend,
    d2d_device_context_GetPrimitiveBlend,
    d2d_device_context_SetUnitMode,
    d2d_device_context_GetUnitMode,
    d2d_device_context_ID2D1DeviceContext_DrawGlyphRun,
    d2d_device_context_DrawImage,
    d2d_device_context_DrawGdiMetafile,
    d2d_device_context_ID2D1DeviceContext_DrawBitmap,
    d2d_device_context_ID2D1DeviceContext_PushLayer,
    d2d_device_context_InvalidateEffectInputRectangle,
    d2d_device_context_GetEffectInvalidRectangleCount,
    d2d_device_context_GetEffectInvalidRectangles,
    d2d_device_context_GetEffectRequiredInputRectangles,
    d2d_device_context_ID2D1DeviceContext_FillOpacityMask,
};

HRESULT d2d_device_context_init(struct d2d_device_context *This,
        ID2D1Device *device_iface, D2D1_DEVICE_CONTEXT_OPTIONS options,
        ID3D10Device *d3d_device)
{
    HRESULT hr;
    ID2D1Factory *factory;
    D2D1_RENDER_TARGET_PROPERTIES desc;
    desc.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
    desc.pixelFormat.format = DXGI_FORMAT_UNKNOWN;
    desc.pixelFormat.alphaMode = D2D1_ALPHA_MODE_UNKNOWN;
    desc.dpiX = 96.0f;
    desc.dpiY = 96.0f;
    desc.usage = D2D1_RENDER_TARGET_USAGE_NONE;
    desc.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;

    if (options == D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS)
        FIXME("D2D1_DEVICE_CONTEXT_OPTIONS ignored.");

    This->ID2D1DeviceContext_iface.lpVtbl = &d2d_device_context_vtbl;
    This->refcount = 1;
    This->device = device_iface;

    ID2D1Device_GetFactory(This->device, &factory);
    hr = d2d_d3d_create_render_target_with_device(factory, d3d_device,
            (IUnknown *)&This->ID2D1DeviceContext_iface,
            &desc, &This->dxgi_target);
    ID2D1Factory_Release(factory);
    if (FAILED(hr))
    {
        WARN("Failed to create base render target, hr %#x.\n", hr);
        return hr;
    }

    ID2D1Device_AddRef(This->device);

    return S_OK;
}
