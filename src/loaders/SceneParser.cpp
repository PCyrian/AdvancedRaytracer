/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include <libconfig.h++>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <map>
#include <string>
#include <memory>

#include "../../include/core/Raytracer.hpp"
#include "loaders/SceneParser.hpp"
#include "rendering/Scene.hpp"
#include "rendering/Camera.hpp"
#include "rendering/Light.hpp"
#include "rendering/Material.hpp"
#include "rendering/Texture.hpp"
#include "../../include/rendering/NoiseTexture.hpp"
#include "rendering/CheckerTexture.hpp"
#include "rendering/ImageTexture.hpp"
#include "primitives/Sphere.hpp"
#include "primitives/Plane.hpp"
#include "primitives/Box.hpp"
#include "primitives/Cylinder.hpp"
#include "primitives/Cone.hpp"
#include "primitives/Triangle.hpp"
#include "primitives/Torus.hpp"
#include "primitives/Sierpinski.hpp"
#include "primitives/Menger.hpp"
#include "primitives/TangleCube.hpp"
#include "geoTransf/Shear.hpp" 

#include "loaders/STLLoader.hpp"

using namespace libconfig;
using namespace RayTracer;
using namespace Math;

// Helpers
static Vector3D parseVec3(const Setting &s, const char* kx="x", const char* ky="y", const char* kz="z") {
    auto get = [&](const char* k)->double {
        if (!s.exists(k)) return 0.0;
        const Setting &v = s.lookup(k);
        return v.getType()==Setting::TypeInt ? (double)(int)v : (double)v;
    };
    return { get(kx), get(ky), get(kz) };
}
static Vector3D parseColor(const Setting &s) {
    Vector3D c = parseVec3(s, "r","g","b");
    return { c.x/255.0, c.y/255.0, c.z/255.0 };
}


static Material parseMaterialDef(const Setting &m, bool debug) {
    Material mat;
    m.lookupValue("name",        mat.name);
    mat.albedo      = parseColor(m.lookup("albedo"));
    m.lookupValue("roughness",   mat.roughness);
    m.lookupValue("metallic",    mat.metallic);
    if (m.exists("specular")) {
        const Setting &sp = m.lookup("specular");
        if (sp.getType() == Setting::TypeGroup)
            mat.specular = parseVec3(sp, "r", "g", "b");
        else {
            double sVal = 0.0;
            m.lookupValue("specular", sVal);
            mat.specular = Vector3D{ sVal, sVal, sVal };
        }
    }
    m.lookupValue("transparency", mat.transparency);
    m.lookupValue("ior",          mat.ior);
    if (m.exists("emission")) {
        mat.emission = parseColor(m.lookup("emission"));
    }
    m.lookupValue("emissionStrength", mat.emissionStrength);

    // Diffuse map: checker or image
    if (m.exists("checker")) {
        const Setting &ch = m.lookup("checker");
        auto c1 = parseColor(ch.lookup("color1"));
        auto c2 = parseColor(ch.lookup("color2"));
        float scale = 1.0f;
        ch.lookupValue("scale", scale);
        mat.diffuseMap = std::make_shared<CheckerTexture>(c1, c2, scale);
    }
    else if (m.exists("texture")) {
        std::string file;
        m.lookupValue("texture", file);
        mat.diffuseMap = std::make_shared<ImageTexture>(file);
    }

    // Noise texture wrapping
    if (m.exists("noise") && mat.diffuseMap) {
        const Setting &ns = m.lookup("noise");
        float noiseScale = 10.0f;
        float noiseStrength = 0.1f;
        ns.lookupValue("scale", noiseScale);
        ns.lookupValue("strength", noiseStrength);
        // Wrap the existing diffuseMap
        mat.diffuseMap = std::make_shared<NoiseTexture>(mat.diffuseMap,
                                                         noiseScale,
                                                         noiseStrength);
    }

    if (debug) {
        std::cerr << "[Debug] Material '"<<mat.name<<"' albedo=("<<mat.albedo.x<<","<<mat.albedo.y<<","<<mat.albedo.z<<") "
                  <<"roughness="<<mat.roughness<<" metallic="<<mat.metallic
                  <<" specular=("<<mat.specular.x<<","<<mat.specular.y<<","<<mat.specular.z<<") "
                  <<"transparency="<<mat.transparency<<" ior="<<mat.ior
                  <<" emission=("<<mat.emission.x<<","<<mat.emission.y<<","<<mat.emission.z<<")"
                  <<" strength="<<mat.emissionStrength<<"\n";
    }
    return mat;
}

static std::shared_ptr<Primitive> ParseShear(const libconfig::Setting &node, std::shared_ptr<Primitive> prim,
    bool debug = false)
{
    if (!node.exists("shear"))
        return prim;

    const Setting &sh = node.lookup("shear");
    double sXY = 0, sXZ = 0, sYX = 0, sYZ = 0, sZX = 0, sZY = 0;

    sh.lookupValue("xy", sXY);
    sh.lookupValue("xz", sXZ);
    sh.lookupValue("yx", sYX);
    sh.lookupValue("yz", sYZ);
    sh.lookupValue("zx", sZX);
    sh.lookupValue("zy", sZY);

    if (debug) {
        std::cerr << "[Debug] shear " << "xy=" << sXY << " xz=" << sXZ << " " << "yx=" << sYX << " yz=" << sYZ << " "
            << "zx=" << sZX << " zy=" << sZY << "\n";
    }
    return std::make_shared<Shear>(prim, sXY, sXZ, sYX, sYZ, sZX, sZY);
}

static Material resolveMaterial(const Setting &s,
                                const std::map<std::string,Material> &mats,
                                const Vector3D &fallback) {
    Material mat;
    std::string name;
    if (s.lookupValue("material", name) && mats.count(name)) {
        mat = mats.at(name);
    } else {
        mat = Material{"default", fallback, 0.0f, 0.0f, {0,0,0}, 0.0f, 1.0f};
    }
    if (s.exists("checker")) {
        const Setting &ch = s.lookup("checker");
        auto c1 = parseColor(ch.lookup("color1"));
        auto c2 = parseColor(ch.lookup("color2"));
        float scale = 1.0f; ch.lookupValue("scale", scale);
        mat.diffuseMap = std::make_shared<CheckerTexture>(c1, c2, scale);
    } else if (s.exists("texture")) {
        std::string file; s.lookupValue("texture", file);
        mat.diffuseMap = std::make_shared<ImageTexture>(file);
    }
    if (s.exists("emission")) {
        mat.emission = parseColor(s.lookup("emission"));
    }
    if (s.exists("emissionStrength")) {
        s.lookupValue("emissionStrength", mat.emissionStrength);
    }
    return mat;
}

static void ParseCamera(const Setting &cam, Scene &scene, bool debug) {
    auto pos = parseVec3(cam.lookup("position"));
    auto rot = cam.exists("rotation") ? parseVec3(cam.lookup("rotation")) : Vector3D{0,0,0};
    double fov = 45.0; cam.lookupValue("fieldOfView", fov);

    int w = 800, h = 600;
    if (cam.exists("resolution")) {
        const Setting &r = cam.lookup("resolution");
        r.lookupValue("width", w);
        r.lookupValue("height", h);
    }

    double aperture = 0.0;
    cam.lookupValue("aperture", aperture);
    double focusDist = 1.0;
    cam.lookupValue("focusDistance", focusDist);

    Vector3D fwd{0,0,-1};
    fwd = fwd.rotateX(rot.x).rotateY(rot.y).rotateZ(rot.z);
    Vector3D up{0,1,0};
    Vector3D right = fwd.cross(up) / fwd.cross(up).length();
    Vector3D realUp = right.cross(fwd) / right.cross(fwd).length();

    scene = Scene(Camera(
            Point3D{pos.x, pos.y, pos.z},
            Point3D{pos.x + fwd.x * focusDist,
                    pos.y + fwd.y * focusDist,
                    pos.z + fwd.z * focusDist},
            realUp,
            fov,
            double(w) / h,
            aperture,
            focusDist,
            h,
            w
    ));
    if (debug) {
        std::cerr << "[Debug] Camera pos=(" << pos.x << "," << pos.y << "," << pos.z << ")"
                << " rot=(" << rot.x << "," << rot.y << "," << rot.z << ")"
                << " fov=" << fov
                << " res=" << w << "x" << h
                << " aperture=" << aperture
                << " focusDist=" << focusDist << "\n";
    }
}

static void ParseSpheres(const Setting &ss, Scene &scene, const std::map<std::string,Material> &mats, bool debug) {
    for (int i=0;i<ss.getLength();++i){
        auto &s=ss[i]; auto c=parseVec3(s);
        double r=1; s.lookupValue("r",r);
        Vector3D col=s.exists("color")?parseColor(s.lookup("color")):Vector3D{1,1,1};
        Material mat=resolveMaterial(s,mats,col);

        std::shared_ptr<Primitive> sphere = std::make_shared<Sphere>(Point3D{c.x, c.y, c.z}, r, mat);
        //Léo tu peux ajouter l'appel de la fonction des rotation ici
        sphere = ParseShear(s, sphere, debug);
        scene.addObject(sphere);
        if (debug) {
            std::cerr<<"[Debug] Sphere center=("<<c.x<<","<<c.y<<","<<c.z<<") r="<<r
                    <<" mat='"<<mat.name<<"'\n";
        }
    }
}

static void ParseBoxes(const Setting &bs, Scene &scene, const std::map<std::string,Material> &mats, bool debug) {
    for(int i=0;i<bs.getLength();++i){
        auto &b=bs[i]; auto mn=parseVec3(b.lookup("min")); auto mx=parseVec3(b.lookup("max"));
        Vector3D col=b.exists("color")?parseColor(b.lookup("color")):Vector3D{1,1,1};
        Material mat=resolveMaterial(b,mats,col);

        std::shared_ptr<Primitive> box = std::make_shared<Box>(Point3D{mn.x,mn.y,mn.z},Point3D{mx.x,mx.y,mx.z},mat);
        box = ParseShear(b, box, debug);
        scene.addObject(box);
        if (debug) {
            std::cerr<<"[Debug] Box min=("<<mn.x<<","<<mn.y<<","<<mn.z<<") max=("<<mx.x<<","<<mx.y<<","<<mx.z<<") "
                    <<"mat='"<<mat.name<<"'\n";
        }
    }
}

static void ParseCylinders(const Setting &cs, Scene &scene, const std::map<std::string,Material> &mats, bool debug) {
    for(int i=0;i<cs.getLength();++i){
        auto &c=cs[i]; auto base=parseVec3(c);
        double r=1,h=1; c.lookupValue("r",r); c.lookupValue("h",h);
        Vector3D col=c.exists("color")?parseColor(c.lookup("color")):Vector3D{1,1,1};
        Material mat=resolveMaterial(c,mats,col);

        std::shared_ptr<Primitive> cylinder = std::make_shared<Cylinder>(Point3D{base.x,base.y,base.z}, 
            Vector3D{0,1,0},r,h,mat);
        cylinder = ParseShear(c, cylinder, debug);
        scene.addObject(cylinder);
        if (debug) {
            std::cerr<<"[Debug] Cylinder base=("<<base.x<<","<<base.y<<","<<base.z<<") r="<<r<<" h="<<h
                    <<" mat='"<<mat.name<<"'\n";
        }
    }
}

static void ParseCones(const Setting &cs, Scene &scene, const std::map<std::string,Material> &mats, bool debug) {
    for(int i=0;i<cs.getLength();++i){
        auto &c=cs[i]; auto base=parseVec3(c);
        double r1=0,r2=0,h=1; c.lookupValue("r1",r1); c.lookupValue("r2",r2); c.lookupValue("h",h);
        if(r1>0&&r2>0) continue;
        Point3D apex; Vector3D axis; double angle=0;
        if(r1==0){ apex=Point3D{base.x,base.y,base.z}; axis={0,1,0}; angle=atan2(r2,h)*180/M_PI; }
        else       { apex=Point3D{base.x,base.y+h,base.z}; axis={0,-1,0}; angle=atan2(r1,h)*180/M_PI; }
        Vector3D col=c.exists("color")?parseColor(c.lookup("color")):Vector3D{1,1,1};
        Material mat=resolveMaterial(c,mats,col);

        std::shared_ptr<Primitive> cone = std::make_shared<Cone>(apex,axis,angle,h,mat);
        cone = ParseShear(c, cone, debug);
        scene.addObject(cone);
        if (debug) {
            std::cerr<<"[Debug] Cone apex=("<<apex.x<<","<<apex.y<<","<<apex.z<<") axis=("<<axis.x<<","<<axis.y<<","<<axis.z<<") "
                    <<"angle="<<angle<<" h="<<h<<" mat='"<<mat.name<<"'\n";
        }
    }
}

static void ParsePlanes(const Setting &ps, Scene &scene, const std::map<std::string,Material> &mats, bool debug) {
    for(int i=0;i<ps.getLength();++i){
        auto &p=ps[i]; std::string ax; p.lookupValue("axis",ax); double pos=0; p.lookupValue("position",pos);
        Point3D point; Vector3D normal;
        if(ax=="X"){ normal={1,0,0}; point={pos,0,0}; }
        else if(ax=="Y"){ normal={0,1,0}; point={0,pos,0}; }
        else             { normal={0,0,1}; point={0,0,pos}; }
        Vector3D col=p.exists("color")?parseColor(p.lookup("color")):Vector3D{1,1,1};
        Material mat=resolveMaterial(p,mats,col);

        std::shared_ptr<Primitive> plane = std::make_shared<Plane>(point,normal,mat);
        plane = ParseShear(p, plane, debug);
        scene.addObject(plane);
        if (debug) {
            std::cerr<<"[Debug] Plane axis="<<ax<<" pos="<<pos<<" mat='"<<mat.name<<"'\n";
        }
    }
}

static void ParseTorus(const Setting &ts, Scene &scene, const std::map<std::string,Material> &mats, bool debug) {
    for(int i=0;i<ts.getLength();++i){
        auto &t=ts[i]; auto c=parseVec3(t.lookup("center")); double R=1,r=0.5;
        t.lookupValue("majorRadius",R); t.lookupValue("minorRadius",r);
        Vector3D col=t.exists("color")?parseColor(t.lookup("color")):Vector3D{1,1,1};
        Material mat=resolveMaterial(t,mats,col);

        std::shared_ptr<Primitive> torus = std::make_shared<Torus>(Point3D{c.x,c.y,c.z},R,r,mat);
        torus = ParseShear(t, torus, debug);
        scene.addObject(torus);
        if (debug) {
            std::cerr<<"[Debug] Torus center=("<<c.x<<","<<c.y<<","<<c.z<<") R="<<R<<" r="<<r
                    <<" mat='"<<mat.name<<"'\n";
        }
    }
}

static void ParseMeshes(const Setting &ms, Scene &scene, const std::map<std::string,Material> &mats, bool debug) {
    for(int i=0;i<ms.getLength();++i){
        auto &m=ms[i]; std::string file; m.lookupValue("file",file);
        auto tr=parseVec3(m.exists("translate")?m.lookup("translate"):m);
        double sc=1; m.lookupValue("scale",sc);
        Vector3D col = m.exists("color")?parseColor(m.lookup("color")):Vector3D{1,1,1};
        Material mat = resolveMaterial(m,mats,col);
        auto mesh = STLLoader::load(file); mesh.transform(tr,sc);

        for (auto &tri : mesh.triangles) {
            // 👑 Use Primitive, not Triangle, so assignment works:
            std::shared_ptr<Primitive> obj =
                std::make_shared<Triangle>(tri.v0, tri.v1, tri.v2, mat);
            obj = ParseShear(m, obj, debug);
            scene.addObject(obj);
        }
        if (debug) {
            std::cerr<<"[Debug] Mesh '"<<file<<"' translate=("<<tr.x<<","<<tr.y<<","<<tr.z<<") "
                    <<"scale="<<sc<<" tris="<<mesh.triangles.size()<<" mat='"<<mat.name<<"'\n";
        }
    }
}

static void ParseLights(const Setting &ls, Scene &scene, bool debug) {
    double amb=0.1,dif=1.0; ls.lookupValue("ambient",amb); ls.lookupValue("diffuse",dif);
    scene.addLight(Light::ambient(amb));
    if (debug)
        std::cerr<<"[Debug] Light ambient="<<amb<<" diffuse="<<dif<<"\n";
    if(ls.exists("point")) for(int i=0;i<ls.lookup("point").getLength();++i){
            auto &pt=ls.lookup("point")[i]; auto p=parseVec3(pt);
            double inten=dif; pt.lookupValue("intensity",inten);
            scene.addLight(Light::point(Point3D{p.x,p.y,p.z},inten));
            if (debug)
                std::cerr<<"[Debug] PointLight @("<<p.x<<","<<p.y<<","<<p.z<<") inten="<<inten<<"\n";
        }
    if(ls.exists("directional")) for(int i=0;i<ls.lookup("directional").getLength();++i){
            auto &d=ls.lookup("directional")[i]; auto dir=parseVec3(d);
            double inten=dif; d.lookupValue("intensity",inten);
            scene.addLight(Light::directional(dir,inten));
            if (debug)
                std::cerr<<"[Debug] DirLight dir=("<<dir.x<<","<<dir.y<<","<<dir.z<<") inten="<<inten<<"\n";
        }
}

static void ParseSierpinski(const Setting &ss, Scene &scene, const std::map<std::string, Material> &mats, bool debug) {
    for (int i = 0; i < ss.getLength(); ++i) {
        const Setting &s = ss[i];
        auto c = parseVec3(s.lookup("center"));
        double size = 1.0; s.lookupValue("size", size);
        int depth = 2;     s.lookupValue("depth", depth);
        Vector3D col = s.exists("color") ? parseColor(s.lookup("color")) : Vector3D{1,1,1};
        Material mat = resolveMaterial(s, mats, col);

        std::shared_ptr<Primitive> sierpinski = std::make_shared<Sierpinski>(Point3D{c.x, c.y, c.z}, size, depth, mat);
        sierpinski = ParseShear(s, sierpinski, debug);
        scene.addObject(sierpinski);
        if (debug) {
            std::cerr << "[Debug] Sierpinski center=(" << c.x << "," << c.y << "," << c.z
                    << ") size=" << size << " depth=" << depth
                    << " mat='" << mat.name << "'\n";
        }
    }
}

static void ParseMenger(const Setting &ss, Scene &scene, const std::map<std::string, Material> &mats, bool debug) {
    for (int i = 0; i < ss.getLength(); ++i) {
        const Setting &s = ss[i];
        auto c = parseVec3(s.lookup("center"));
        double size = 1.0; s.lookupValue("size", size);
        int depth = 2;     s.lookupValue("depth", depth);
        Vector3D col = s.exists("color") ? parseColor(s.lookup("color")) : Vector3D{1,1,1};
        Material mat = resolveMaterial(s, mats, col);

        std::shared_ptr<Primitive> menger = std::make_shared<Menger>(Point3D{c.x, c.y, c.z}, size, depth, mat);
        menger = ParseShear(s, menger, debug);
        scene.addObject(menger);
        if (debug) {
            std::cerr << "[Debug] Menger center=(" << c.x << "," << c.y << "," << c.z
                    << ") size=" << size << " depth=" << depth
                    << " mat='" << mat.name << "'\n";
        }
    }
}

static void ParseTangleCube(const Setting &tc, Scene &scene, const std::map<std::string,Material> &mats, bool debug)
{
    for (int i = 0; i < tc.getLength(); ++i) {
        const Setting &t = tc[i];
        Vector3D vc = parseVec3(t.lookup("center"));
        Point3D center{ vc.x, vc.y, vc.z };
        double scale = 1.0;
        t.lookupValue("scale", scale);
        Vector3D defaultCol = t.exists("color") ? parseColor(t.lookup("color")) : Vector3D{1,1,1};
        Material mat = resolveMaterial(t, mats, defaultCol);

        std::shared_ptr<Primitive> tangleC = std::make_shared<TangleCube>(center, scale, mat);
        tangleC = ParseShear(t, tangleC, debug);
        scene.addObject(tangleC);
        if (debug) {
            std::cerr << "[Debug] TangleCube center(" << center.x << "," << center.y << "," << center.z
            << ") scale=" << scale << " mat='" << mat.name << "'\n";
        }
    }
}

bool RayTracer::loadScene(const std::string &file, Scene &scene, bool debug) {
    Config cfg;

    try { 
        cfg.readFile(file.c_str());

        // Camera
        ParseCamera(cfg.lookup("camera"), scene, debug);

        // Skybox (optional)
        if (cfg.exists("skybox")) {
            std::string skyboxFile;
            cfg.lookupValue("skybox", skyboxFile);
            if (!skyboxFile.empty()) {
                if (!Skybox::initializeSkybox(skyboxFile)) {
                    std::cerr << skyboxFile << std::endl;
                    return false;
                }
            }
        }

        // Materials|
        std::map<std::string,Material> mats;
        if(cfg.exists("materials")) {
            for(int i=0;i<cfg.lookup("materials").getLength();++i){
                Material m = parseMaterialDef(cfg.lookup("materials")[i], debug);
                mats[m.name] = m;
            }
        }
    
        // Primitives
        if(cfg.exists("primitives")){
            auto &pr = cfg.lookup("primitives");
    
            if(pr.exists("spheres"))
                ParseSpheres(pr.lookup("spheres"),    scene,mats, debug);
            if(pr.exists("boxes"))
                ParseBoxes(pr.lookup("boxes"),      scene,mats, debug);
            if(pr.exists("cylinders"))
                ParseCylinders(pr.lookup("cylinders"),scene,mats, debug);
            if(pr.exists("cones"))
                ParseCones(pr.lookup("cones"),      scene,mats, debug);
            if(pr.exists("torus"))
                ParseTorus(pr.lookup("torus"),      scene,mats, debug);
            if(pr.exists("meshes"))
                ParseMeshes(pr.lookup("meshes"),     scene,mats, debug);
            if(pr.exists("sierpinski"))
                ParseSierpinski(pr.lookup("sierpinski"),     scene,mats, debug);
            if(pr.exists("menger"))
                ParseMenger(pr.lookup("menger"),     scene,mats, debug);
            if(pr.exists("tangleCube"))
                ParseTangleCube(pr.lookup("tangleCube"),     scene,mats, debug);
        }
    
        // Planes (top-level)
        if(cfg.exists("planes"))
            ParsePlanes(cfg.lookup("planes"), scene, mats, debug);
    
        // Lights
        if(cfg.exists("lights")) ParseLights(cfg.lookup("lights"), scene, debug);
    
        return true;
    } catch(const FileIOException &e) {
        std::string error_mesage("I/O error reading " + file);
        throw Raytracer::Error(error_mesage);
    } catch(const libconfig::ParseException &e) {
        std::string msg = "Parse error in file " + std::string(e.getFile()) +
                          " at line " + std::to_string(e.getLine()) +
                          ": " + e.getError();
        throw Raytracer::Error(msg);
    } catch(const libconfig::SettingNotFoundException &e) {
        throw Raytracer::Error("Missing setting: " + std::string(e.what()));
    } catch (const libconfig::SettingTypeException &e) {
        throw Raytracer::Error("Wrong type for setting: " + std::string(e.what()));
    } catch (const std::exception &e) {
        throw Raytracer::Error("Unexpected error while loading scene: " + std::string(e.what()));
    }
}
