
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2013 Esteban Tovagliari, Jupiter Jazz Limited
// Copyright (c) 2014-2015 Esteban Tovagliari, The appleseedhq Organization
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#ifndef APPLESEED_RENDERER_KERNEL_RENDERING_RENDERERSERVICES_H
#define APPLESEED_RENDERER_KERNEL_RENDERING_RENDERERSERVICES_H

// appleseed.foundation headers.
#include "foundation/core/concepts/noncopyable.h"
#include "foundation/platform/compiler.h"

// OSL headers.
#include "foundation/platform/oslheaderguards.h"
BEGIN_OSL_INCLUDES
#include "OSL/oslversion.h"
#include "OSL/rendererservices.h"
END_OSL_INCLUDES

// OpenImageIO headers.
#include "OpenImageIO/texture.h"

// Boost headers.
#include "boost/unordered_map.hpp"

// Standard headers.
#include <string>

// Forward declarations.
namespace renderer  { class Camera; }
namespace renderer  { class Project; }
namespace renderer  { class TextureStore; }
namespace renderer  { class TraceContext; }

namespace renderer
{

//
// appleseed's OSL RendererServices.
//

class RendererServices
  : public OSL::RendererServices
{
  public:
    // Constructor.
    RendererServices(
        const Project&          project,
        OIIO::TextureSystem&    texture_sys,
        TextureStore&           texture_store);

    // Initialize before rendering starts.
    void initialize();

    // Return a pointer to the texture system (if available).
    virtual OIIO::TextureSystem* texturesys() const APPLESEED_OVERRIDE;

    // Filtered 2D texture lookup for a single point.
    virtual bool texture(
        OSL::ustring            filename,
        OSL::TextureOpt&        options,
        OSL::ShaderGlobals*     sg,
        float                   s,
        float                   t,
        float                   dsdx,
        float                   dtdx,
        float                   dsdy,
        float                   dtdy,
#if OSL_LIBRARY_VERSION_CODE <= 10600
        float*                  result
#else
        int                     nchannels,
        float*                  result,
        float*                  dresultds,
        float*                  dresultdt
#endif
        ) APPLESEED_OVERRIDE;

    // Filtered 3D texture lookup for a single point.
    //
    // P is the volumetric texture coordinate; dPd{x,y,z} are the
    // differentials of P in some canonical directions x, y, and z.
    // The choice of x,y,z are not important to the implementation; it
    // can be any imposed 3D coordinates, such as pixels in screen
    // space and depth along the ray, etc.
    //
    // Return true if the file is found and could be opened, otherwise
    // return false.
    virtual bool texture3d(
        OSL::ustring            filename,
        OSL::TextureOpt&        options,
        OSL::ShaderGlobals*     sg,
        const OSL::Vec3&        P,
        const OSL::Vec3&        dPdx,
        const OSL::Vec3&        dPdy,
        const OSL::Vec3&        dPdz,
#if OSL_LIBRARY_VERSION_CODE <= 10600
        float*                  result
#else
        int                     nchannels,
        float*                  result,
        float*                  dresultds,
        float*                  dresultdt
#endif
        ) APPLESEED_OVERRIDE;

    // Filtered environment lookup for a single point.
    //
    // R is the directional texture coordinate; dRd[xy] are the
    // differentials of R in canonical directions x, y.
    //
    // Return true if the file is found and could be opened, otherwise
    // return false.
    virtual bool environment(
        OSL::ustring            filename,
        OSL::TextureOpt&        options,
        OSL::ShaderGlobals*     sg,
        const OSL::Vec3&        R,
        const OSL::Vec3&        dRdx,
        const OSL::Vec3&        dRdy,
#if OSL_LIBRARY_VERSION_CODE <= 10600
        float*                  result
#else
        int                     nchannels,
        float*                  result,
        float*                  dresultds,
        float*                  dresultdt
#endif
        ) APPLESEED_OVERRIDE;

    // Get information about the given texture.  Return true if found
    // and the data has been put in* data.  Return false if the texture
    // doesn't exist, doesn't have the requested data, if the data
    // doesn't match the type requested. or some other failure.
    virtual bool get_texture_info(
        OSL::ShaderGlobals*     sg,
        OSL::ustring            filename,
        int                     subimage,
        OSL::ustring            dataname,
        OSL::TypeDesc           datatype,
        void*                   data) APPLESEED_OVERRIDE;

    // Get the 4x4 matrix that transforms by the specified
    // transformation at the given time.  Return true if ok, false
    // on error.
    virtual bool get_matrix(
        OSL::ShaderGlobals*     sg,
        OSL::Matrix44&          result,
        OSL::TransformationPtr  xform,
        float                   time) APPLESEED_OVERRIDE;

    // Get the 4x4 matrix that transforms by the specified
    // transformation at the given time.  Return true if ok, false on
    // error.  The default implementation is to use get_matrix and
    // invert it, but a particular renderer may have a better technique
    // and overload the implementation.
    virtual bool get_inverse_matrix(
        OSL::ShaderGlobals*     sg,
        OSL::Matrix44&          result,
        OSL::TransformationPtr  xform,
        float                   time) APPLESEED_OVERRIDE;

    // Get the 4x4 matrix that transforms by the specified
    // transformation.  Return true if ok, false on error.  Since no
    // time value is given, also return false if the transformation may
    // be time-varying.
    virtual bool get_matrix(
        OSL::ShaderGlobals*     sg,
        OSL::Matrix44&          result,
        OSL::TransformationPtr  xform) APPLESEED_OVERRIDE;

    // Get the 4x4 matrix that transforms by the specified
    // transformation.  Return true if ok, false on error.  Since no
    // time value is given, also return false if the transformation may
    // be time-varying.  The default implementation is to use
    // get_matrix and invert it, but a particular renderer may have a
    // better technique and overload the implementation.
    virtual bool get_inverse_matrix(
        OSL::ShaderGlobals*     sg,
        OSL::Matrix44&          result,
        OSL::TransformationPtr  xform) APPLESEED_OVERRIDE;

    // Get the 4x4 matrix that transforms points from the named
    // 'from' coordinate system to "common" space at the given time.
    // Returns true if ok, false if the named matrix is not known.
    virtual bool get_matrix(
        OSL::ShaderGlobals*     sg,
        OSL::Matrix44&          result,
        OIIO::ustring           from,
        float                   time) APPLESEED_OVERRIDE;

    // Get the 4x4 matrix that transforms points from "common" space to
    // the named 'to' coordinate system to at the given time.  The
    // default implementation is to use get_matrix and invert it, but a
    // particular renderer may have a better technique and overload the
    // implementation.
    virtual bool get_inverse_matrix(
        OSL::ShaderGlobals*     sg,
        OSL::Matrix44&          result,
        OSL::ustring            to,
        float                   time) APPLESEED_OVERRIDE;

    // Get the 4x4 matrix that transforms 'from' to "common" space.
    // Since there is no time value passed, return false if the
    // transformation may be time-varying(as well as if it's not found
    // at all).
    virtual bool get_matrix(
        OSL::ShaderGlobals*     sg,
        OSL::Matrix44&          result,
        OIIO::ustring           from) APPLESEED_OVERRIDE;

    // Get the 4x4 matrix that transforms points from "common" space to
    // the named 'to' coordinate system.  Since there is no time value
    // passed, return false if the transformation may be time-varying
    // (as well as if it's not found at all).  The default
    // implementation is to use get_matrix and invert it, but a
    // particular renderer may have a better technique and overload the
    // implementation.
    virtual bool get_inverse_matrix(
        OSL::ShaderGlobals*     sg,
        OSL::Matrix44&          result,
        OSL::ustring            to) APPLESEED_OVERRIDE;

    // Transform points Pin[0..npoints-1] in named coordinate system
    // 'from' into 'to' coordinates, storing the result in Pout[] using
    // the specified vector semantic (POINT, VECTOR, NORMAL).  The
    // function returns true if the renderer correctly transformed the
    // points, false if it failed (for example, because it did not know
    // the name of one of the coordinate systems).  A renderer is free
    // to not implement this, in which case the default implementation
    // is simply to make appropriate calls to get_matrix and
    // get_inverse_matrix.  The existance of this method is to allow
    // some renderers to provide transformations that cannot be
    // expressed by a 4x4 matrix.
    //
    // If npoints == 0, the function should just return true if a
    // known nonlinear transformation is available to transform points
    // between the two spaces, otherwise false.  (For this calling
    // pattern, sg, Pin, Pout, and time will not be used and may be 0.
    // As a special case, if from and to are both empty strings, it
    // returns true if *any* nonlinear transformations are supported,
    // otherwise false.
    //
    // Note to RendererServices implementations: just return 'false'
    // if there isn't a special nonlinear transformation between the
    // two spaces.
    virtual bool transform_points(
        OSL::ShaderGlobals*         sg,
        OSL::ustring                from,
        OSL::ustring                to,
        float                       time,
        const OSL::Vec3*            Pin,
        OSL::Vec3*                  Pout,
        int                         npoints,
        OSL::TypeDesc::VECSEMANTICS vectype) APPLESEED_OVERRIDE;

    // Immediately trace a ray from P in the direction R.  Return true
    // if anything hit, otherwise false.
    bool trace(
        TraceOpt&               options,
        OSL::ShaderGlobals*     sg,
        const OSL::Vec3&        P,
        const OSL::Vec3&        dPdx,
        const OSL::Vec3&        dPdy,
        const OSL::Vec3&        R,
        const OSL::Vec3&        dRdx,
        const OSL::Vec3&        dRdy) APPLESEED_OVERRIDE;

    // Get the named message from the renderer and if found then
    // write it into 'val'.  Otherwise, return false.  This is only
    // called for "sourced" messages, not ordinary intra-group messages.
    bool getmessage(
        OSL::ShaderGlobals*     sg,
        OIIO::ustring           source,
        OIIO::ustring           name,
        OIIO::TypeDesc          type,
        void*                   val,
        bool                    derivatives) APPLESEED_OVERRIDE;

    // Get the named attribute from the renderer and if found then
    // write it into 'val'.  Otherwise, return false.  If no object is
    // specified (object == ustring()), then the renderer should search *first*
    // for the attribute on the currently shaded object, and next, if
    // unsuccessful, on the currently shaded "scene".
    //
    // Note to renderers: if renderstate is NULL, that means
    // get_attribute is being called speculatively by the runtime
    // optimizer, and it doesn't know which object the shader will be
    // run on. Be robust to this situation, return 'true' (retrieve the
    // attribute) if you can (known object and attribute name), but
    // otherwise just fail by returning 'false'.
    virtual bool get_attribute(
        OSL::ShaderGlobals*     sg,
        bool                    derivatives,
        OIIO::ustring           object,
        OIIO::TypeDesc          type,
        OIIO::ustring           name,
        void*                   val) APPLESEED_OVERRIDE;

    // Similar to get_attribute();  this method will return the 'index'
    // element of an attribute array.
    virtual bool get_array_attribute(
        OSL::ShaderGlobals*     sg,
        bool                    derivatives,
        OIIO::ustring           object,
        OIIO::TypeDesc          type,
        OIIO::ustring           name,
        int                     index,
        void*                   val) APPLESEED_OVERRIDE;

    // Get the named user-data from the current object and write it into
    // 'val'. If derivatives is true, the derivatives should be written into val
    // as well. Return false if no user-data with the given name and type was found.
    virtual bool get_userdata(
        bool                    derivatives,
        OIIO::ustring           name,
        OIIO::TypeDesc          type,
        OSL::ShaderGlobals*     sg,
        void*                   val) APPLESEED_OVERRIDE;

    // Does the current object have the named user-data associated with it?
    virtual bool has_userdata(
        OIIO::ustring           name,
        OIIO::TypeDesc          type,
        OSL::ShaderGlobals*     sg) APPLESEED_OVERRIDE;

  private:
    // This code based on OSL's test renderer.
    typedef bool (RendererServices::*AttrGetterFun)(
        OSL::ShaderGlobals*     sg,
        bool                    derivs,
        OIIO::ustring           object,
        OIIO::TypeDesc          type,
        OIIO::ustring           name,
        void*                   val) const;

    typedef boost::unordered_map<OIIO::ustring, AttrGetterFun, OIIO::ustringHash> AttrGetterMapType;

    #define DECLARE_ATTR_GETTER(name)       \
        bool get_##name(                    \
            OSL::ShaderGlobals* sg,         \
            bool                derivs,     \
            OIIO::ustring       object,     \
            OIIO::TypeDesc      type,       \
            OIIO::ustring       name,       \
            void*               val) const

    // Object attributes.
    DECLARE_ATTR_GETTER(object_instance_id);
    DECLARE_ATTR_GETTER(object_instance_index);
    DECLARE_ATTR_GETTER(assembly_instance_id);

    // Camera attributes.
    DECLARE_ATTR_GETTER(camera_resolution);
    DECLARE_ATTR_GETTER(camera_projection);
    DECLARE_ATTR_GETTER(camera_fov);
    DECLARE_ATTR_GETTER(camera_pixelaspect);
    DECLARE_ATTR_GETTER(camera_clip);
    DECLARE_ATTR_GETTER(camera_clip_near);
    DECLARE_ATTR_GETTER(camera_clip_far);
    DECLARE_ATTR_GETTER(camera_shutter);
    DECLARE_ATTR_GETTER(camera_shutter_open);
    DECLARE_ATTR_GETTER(camera_shutter_close);
    DECLARE_ATTR_GETTER(camera_screen_window);

    // Ray attributes.
    DECLARE_ATTR_GETTER(ray_depth);
    DECLARE_ATTR_GETTER(ray_length);
    DECLARE_ATTR_GETTER(ray_ior);

    #undef DECLARE_ATTR_GETTER

    static void clear_attr_derivatives(
        bool                    derivs,
        const OIIO::TypeDesc&   type,
        void*                   val);

    static void log_error(const std::string& message);

    const Project&              m_project;
    const Camera*               m_camera;
    OIIO::TextureSystem&        m_texture_sys;
    const TraceContext&         m_trace_context;
    TextureStore&               m_texture_store;
    AttrGetterMapType           m_global_attr_getters;
    OIIO::ustring               m_cam_projection_str;
};

}       // namespace renderer

#endif  // !APPLESEED_RENDERER_KERNEL_RENDERING_RENDERERSERVICES_H
